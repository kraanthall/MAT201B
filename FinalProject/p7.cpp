// Ryan Millett
// MAT201B-2023

#include "l-system-backend/parser.hpp"  // pulls in all the necessary headers

#include "al/app/al_App.hpp"
#include "al/app/al_GUIDomain.hpp"

#include "al/app/al_DistributedApp.hpp"
#include "al/math/al_Random.hpp"
#include "al/scene/al_DistributedScene.hpp"

using namespace al;

#include <fstream>
#include <iostream>
#include <utility>
using namespace std;

using namespace gam;

const int MAX_VERTEX_COUNT{500000};
const int MAX_N{72};  // max number of iterations for main system
int n{0};             // current number of iterations for main system

double r() { return rnd::uniformS(); }
RGB c() { return RGB(rnd::uniform(), rnd::uniform(), rnd::uniform()); }

struct Axes {
  void draw(Graphics &g) {
    Mesh mesh(Mesh::LINES);
    // x axis
    mesh.vertex(0, 0, 0);
    // mesh.color(1, 1, 1);  // white
    mesh.color(1, 0, 0);
    mesh.vertex(1, 0, 0);
    mesh.color(1, 0, 0);

    // y axis
    mesh.vertex(0, 0, 0);
    // mesh.color(1, 1, 1);  // white
    mesh.color(0, 1, 0);
    mesh.vertex(0, 1, 0);
    mesh.color(0, 1, 0);

    // z axis
    mesh.vertex(0, 0, 0);
    // mesh.color(1, 1, 1);  // white
    mesh.color(0, 0, 1);
    mesh.vertex(0, 0, 1);
    mesh.color(0, 0, 1);

    g.draw(mesh);
  }
};

// --------------------------------------------------------------
// 0. GET THE TYPE FOR THE CURRENT L-SYSTEM
// --------------------------------------------------------------
// This is the type of the main system.
// The main l-system will work like an interconnected root network 
// where ertain vertices in the mesh can generate branches made
// of different L-Systems.
LSystemType currentSystemType{LSystemType::BOURKE_ALGAE_2};
LSystem currentSystem{TYPE_DEFS.at(currentSystemType)};
al::Mesh currentSystemMesh(al::Mesh::LINES);

struct AlloApp : public DistributedApp {
  // ParameterChoice lsysType{"L-System Type", "", 0};
  ParameterInt currentSystemIndex{"L-System", "", 1, "", 0, 4};
  ParameterInt generations{"Generations", "", 0, "", 0, MAX_N};
  ParameterInt indices{"indices", "", 0, "", 0, MAX_VERTEX_COUNT};
//   Parameter epsilon{"Epsilon", "", 0.000000001, "", 0.0001, 0.1};
//   Parameter randomness{"Randomness", "", 0.000000001, "", 0.0, 1.0};
  // Parameter pointSize{"/pointSize", "", 1.0, 0.0, 2.0};

  // get vector strings for each L-System type from the TYPE_NAMES map
  // std::vector<string> typeNames;
  // for (auto &type : TYPE_NAMES) {
  //   // typeNames.push_back(type.second);
  //   std::cout << type.second << std::endl;
  // }
  // lsysType.setElements(typeNames);

  Axes axes;

  std::string currentSystemString;

  void onInit() override {
    // set up GUI
    auto GUIdomain = GUIDomain::enableGUI(defaultWindowDomain());
    auto &gui = GUIdomain->newGUI();
    gui.add(currentSystemIndex);
    gui.add(generations);
    gui.add(indices);
    // gui.add(epsilon);
    // gui.add(randomness);
  }

  void onCreate() override {
    nav().pos(0, 0, 10); 
    nav().quat(Quatd(0, 0, 0, 1));
  }

  // bool stepOn = false;
    bool onKeyDown(Keyboard const &k) {
        if (k.key() == ' ') {  // Start a new sequence through tree
          
        }
        return true;
  }
  
  float angle = 0;
  float time = 0.f;
  // float interval = 1.f;
  int index = 0;
  int n_prev = 0;
  int sys_prev = 0;
  int sys_index = 1;
  Vec3f avg = Vec3f(0, 0, 0);
  void onAnimate(double dt) override {
    scene.update(dt);
    
    sys_prev = sys_index;
    sys_index = currentSystemIndex.get();
    
    n_prev = n;
    n = generations.get();
  
    if (n == n_prev && sys_prev == sys_index) {
      nav().faceToward(avg);
      return;
    }

    // --------------------------------------------------------------
    // 0. GET THE TYPE FOR THE CURRENT L-SYSTEM

    if (sys_index != sys_prev) {
      n = 0;
      generations.set(0);
    }

    switch (sys_index) {
      case 0:
        currentSystemType = LSystemType::BOURKE_BUSH_2;
        break;
      case 1:
        currentSystemType = LSystemType::BOURKE_ALGAE_2;
        break;
      case 2:
        currentSystemType = LSystemType::BOURKE_WEED;
        break;
      case 3:
        currentSystemType = LSystemType::BOURKE_CRYSTAL;
        break;
      case 4:
        currentSystemType = LSystemType::BOURKE_LEAF;
        break;
    }
    currentSystem = TYPE_DEFS.at(currentSystemType);

    // --------------------------------------------------------------
    // 1. GENERATE THE CURRENT L-SYSTEM STRING
    // --------------------------------------------------------------

    currentSystemString = generateString(currentSystem, n);//generations.get());  // This only generates the string which will be parsed later.
    // cout << "main l-sys type: " << TYPE_NAMES.at(currentSystemType) << endl;
    // cout << "iterations: " << n << endl;
    // cout << "string:\n" << currentSystemString << endl;

    // --------------------------------------------------------------
    // 2. BUILD THE CURRENT L-SYSTEM VERTICES
    // --------------------------------------------------------------
    currentSystemMesh.reset();
    
    struct State : al::Pose {
        al::Color color;

        float currentAngle = currentSystem.angle;
        float currentLength = currentSystem.length;
        int depth = 0;

        State(const al::Vec3f& position, const al::Color& color)
            : al::Pose(position), color(color)
        {}
    };

    std::vector<State> state;  // push_back / pop_back
    // std::vector<al::Pose> state;  // push_back / pop_back
    struct ColorQueue {
        std::vector<al::Color> colors;
        int index = 0;

        ColorQueue(const std::vector<al::Color>& colors) : colors(colors) {}

        al::Color& operator[](int i) {
            return colors[(index + i) % colors.size()];
        }

        al::Color& next() {
            index = (index + 1) % colors.size();
            return colors[index];
        }
    };
    // circular queue of green and green/brown colors
    ColorQueue colorQueue{{al::Color(0, rnd::uniform(0.67, 1.0), 0), al::Color(rnd::uniform(0.33, 0.67), rnd::uniform(0.3, 0.5), 0)}};

    state.push_back(State{al::Vec3f(0, 0, 0), al::Color(0.5, 0.3, 0.1)});
    int maxDepth = state.back().depth;
    float alpha = 0.1;
    float epsilon = 0.00001;
    for (char c : currentSystemString) {

      if (currentSystemMesh.vertices().size() > MAX_VERTEX_COUNT) {
        cout << "max vertices reached" << endl;
        break;
      }

      if (state.back().depth > maxDepth) {
          maxDepth = state.back().depth;
      }
      if (c == 'F') {  // Move forward by `LSystem.length` drawing a line
        // point a
        currentSystemMesh.vertex(state.back().pos());
        currentSystemMesh.color(state.back().color);
        // move forward
        state.back().pos() += state.back().uf() * 0.01;//(1.f - alpha);//state.back().currentLength;
        // point b
        currentSystemMesh.vertex(state.back().pos());
        currentSystemMesh.color(state.back().color);
        // update draw scale (as new branches grow, they get smaller)
        // state.back().currentLength *= currentSystem.scaleFactor;
        alpha -= epsilon;
      } else if (c == '+') {  // Turn left by `LSystem.angle`
          state.back().currentAngle += currentSystem.angle;
          state.back().faceToward(state.back().uu(), 0.5*currentSystem.angle/90.f);
          state.back().faceToward(state.back().ur(), currentSystem.angle/90.f);
      } else if (c == '>') {
        state.back().currentLength /= currentSystem.scaleFactor;
      } else if (c == '>') {
        state.back().currentLength *= currentSystem.scaleFactor;
      } else if (c == '-') {  // Turn right by `LSystem.currentAngle`
          state.back().currentAngle -= currentSystem.angle;
          state.back().faceToward(state.back().uu(), -0.5*currentSystem.angle/90.f);
          state.back().faceToward(state.back().ur(), currentSystem.angle/-90.f);
      } else if (c == '[') {  // CHANGE CURRENT BRANCH
          // Push current state onto stack
          ++state.back().depth;
          state.push_back(state.back());
          // rotate through color queue
          state.back().color = colorQueue.next();
          // state.back().color = al::Color(0, 1, 0);
      } else if (c == ']') {  // RESTORE PREVIOUS BRANCH
          // Pop previous state from stack
          state.pop_back();
          --state.back().depth;
          // state.back().color = al::Color(0, 0, 1);
      }      
    }
    currentSystemMesh.compress();
    indices.set(currentSystemMesh.indices().size());

    // average of each vertex in the mesh
    avg = Vec3f(0, 0, 0);
    for (auto &v : currentSystemMesh.vertices()) {
        avg += v;
    }

    avg /= currentSystemMesh.vertices().size();
    // std::cout << "avg: " << avg << std::endl;
    // nav().smooth(0.5);
    // nav().faceToward(avg);
    nav().faceToward(avg);
  }

  void onDraw(Graphics &g) override {
    g.clear(0);
    g.meshColor();

    g.rotate(angle, 0, 1, 0);
    // axes.draw(g);

    
    // std:: cout << "max depth: " << maxDepth << std::endl;
    // --------------------------------------------------------------
    // 3. RENDER THE CURRENT L-SYSTEM VERTICES
    // --------------------------------------------------------------
    g.draw(currentSystemMesh);

    scene.render(g);  // Render graphics
  }
  DistributedScene scene{TimeMasterMode::TIME_MASTER_CPU};
};

int main() {
  AlloApp app;
  app.configureAudio(48000, 512, 2, 0);
  Domain::master().spu(
      app.audioIO().framesPerSecond());  // tell Gamma the playback rate
  app.start();
}
