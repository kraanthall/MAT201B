// Ryan Millett
// MAT201B-2023

#include "l-system-backend/parser.hpp"  // pulls in all the necessary headers

#include "al/app/al_App.hpp"
#include "al/app/al_GUIDomain.hpp"

#include "Gamma/DFT.h"         // STFT
#include "Gamma/Envelope.h"    //Env<2>
#include "Gamma/Oscillator.h"  // Sine<>
#include "al/app/al_DistributedApp.hpp"
#include "al/math/al_Random.hpp"
#include "al/scene/al_DistributedScene.hpp"

using namespace al;

#include <fstream>
#include <iostream>
#include <utility>
using namespace std;

using namespace gam;

const int N{72};  // number of iterations for main system

// frequency ratios for whole tone scale
float tonic = 440 / 2;
float ratios[12] = {1.0, 1.059463, 1.122462, 1.189207, 1.259921, 1.334840,
                    1.414214, 1.498307, 1.587401, 1.681793, 1.781797, 1.887749};

double r() { return rnd::uniformS(); }
RGB c() { return RGB(rnd::uniform(), rnd::uniform(), rnd::uniform()); }

template <typename T>
T mtof(T m) {
  return 440 * pow(2, (m - 69) / 12);
}

template <typename T>
T dbtoa(T db) {
  return pow(10, db / 20);
}

struct SimpleVoice : PositionedVoice {
  Parameter frequency{"Frequency"};
  Parameter modulation{"Modulation"};
  Parameter index{"Index"};
  gam::Sine<> carrier;
  gam::Sine<> modulator;
  gam::Env<2> envelope;
  gam::STFT stft{1024, 1024 / 2, 0, gam::HANN, gam::COMPLEX};
  float amplitude{0};
  Mesh audioMesh;

  int counter{0};

  SimpleVoice() {
    registerParameters(frequency, modulation, index);

    envelope.levels(0, 1, 0);
    envelope.lengths(0.01, 0.667);

    audioMesh.primitive(Mesh::LINE_STRIP);
    for (int i = 0; i < stft.numBins(); i++) {
      audioMesh.vertex((float)i / stft.numBins(), 0);
    }
  }

  virtual void update(double dt) override {
    if (mIsReplica) return;
    // do things in the primary instance...
    amplitude = envelope.value();
  }

  virtual void onProcess(AudioIOData &io) override {
    while (io()) {
      // modulator.freq(mtof(modulation.get()));
      modulator.freq(modulation.get());
      carrier.freq(frequency.get() + mtof(index.get()) * modulator());
      float fm = carrier();
      if (stft(fm)) {
        for (int i = 0; i < stft.numBins(); i++) {
          audioMesh.vertices()[i].y = log(1 + stft.bin(i).mag());
        }
      }
      io.out(0) += fm * envelope() * 0.1;
    }
    if (envelope.done()) free();  // frees this voice
  }

  virtual void onProcess(Graphics &g) override {
    g.color(amplitude);
    g.draw(audioMesh);
  }

  virtual void onTriggerOn() override {
    envelope.reset();

    double f = tonic * ratios[(int)rnd::uniform(12)];
    double m = f / rnd::uniformS(tonic);

    frequency.set(tonic);
    modulation.set(m);
    index.set(rnd::uniform(127.0f));

    tonic *= pow(2, 66.67 / 1200.0);
    counter = (counter + 1) % N;
  }
};

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

LSystemType MAIN_LSYS_TYPE{LSystemType::BOURKE_ALGAE_2};
// LSystemType MAIN_LSYS_TYPE{LSystemType::BOURKE_BUSH_2};


// --------------------------------------------------------------
// 0. GET THE L-SYSTEM TYPE FOR THE MAIN SYSTEM
// --------------------------------------------------------------
// This is the type of the main system.
// The main l-system will work like an interconnected root network 
// where ertain vertices in the mesh can generate branches made
// of different L-Systems.
LSystem mainSystem{TYPE_DEFS.at(MAIN_LSYS_TYPE)};
Mesh mainSystemMesh(Mesh::LINES);

Mesh branchMesh(Mesh::LINES);


struct AlloApp : public DistributedApp {
//   Parameter timeStep{"Time Step", "", 0.33f, "", 0.01, 3.0};
//   Parameter epsilon{"Epsilon", "", 0.000000001, "", 0.0001, 0.1};
//   Parameter randomness{"Randomness", "", 0.000000001, "", 0.0, 1.0};
  // Parameter pointSize{"/pointSize", "", 1.0, 0.0, 2.0};
  Axes axes;

  std::string mainSystemString;

  std::vector<Vec3f> pos;
  std::vector<RGB> colors;


  struct StepState {
    // Vec3f startPoint;
    // RGB startColor;
    std::vector<Vec3f> path;
    // std::vector<RGB> cVec;
    double timeStep;
    double elapsedTime;

    // constructor
    StepState(const std::vector<Vec3f> &pVec, double t) : path(pVec), timeStep(t), elapsedTime(0.0) {}

  };
  std::vector<StepState> stepStates;


  void onInit() override {
    // set up GUI
    auto GUIdomain = GUIDomain::enableGUI(defaultWindowDomain());
    auto &gui = GUIdomain->newGUI();
    // gui.add(timeStep);
    // gui.add(epsilon);
    // gui.add(randomness);
  }

  void onCreate() override {
    // --------------------------------------------------------------
    // 1. GENERATE THE MAIN L-SYSTEM STRING
    // --------------------------------------------------------------
    mainSystemString = generateString(mainSystem, N);  // This only generates the string which will be parsed later.
    cout << "main l-sys type: " << TYPE_NAMES.at(MAIN_LSYS_TYPE) << endl;
    cout << "iterations: " << N << endl;
    cout << "string:\n" << mainSystemString << endl;

    // --------------------------------------------------------------
    // 2. BUILD THE MAIN L-SYSTEM VERTICES
    // --------------------------------------------------------------
    
    struct State : al::Pose {
        al::Color color;

        float currentAngle = mainSystem.angle;
        float currentLength = mainSystem.length;

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
    for (char c : mainSystemString) {
      if (c == 'F') {  // Move forward by `LSystem.length` drawing a line
        // point a
        mainSystemMesh.vertex(state.back().pos());
        mainSystemMesh.color(state.back().color);
        // move forward
        state.back().pos() += state.back().uf() * 0.01;//state.back().currentLength;
        // point b
        mainSystemMesh.vertex(state.back().pos());
        mainSystemMesh.color(state.back().color);
        // update draw scale (as new branches grow, they get smaller)
        state.back().currentLength *= mainSystem.scaleFactor;
      } else if (c == '+') {  // Turn left by `LSystem.angle`
          state.back().currentAngle += mainSystem.angle;
          // state.back().faceToward(state.back().uu(), s);//90./state.back().currentAngle);
          state.back().faceToward(state.back().uu(), 0.1);//90./state.back().currentAngle);
          state.back().faceToward(state.back().ur(), mainSystem.angle/90.f);
      } else if (c == '-') {  // Turn right by `LSystem.currentAngle`
          state.back().currentAngle -= mainSystem.angle;
          // state.back().faceToward(state.back().uu(), -1*rnd::uniform(0.41));//90./state.back().currentAngle);
          state.back().faceToward(state.back().uu(), -0.1);//90./state.back().currentAngle);
          state.back().faceToward(state.back().ur(), mainSystem.angle/-90.f);
      } else if (c == '[') {  // CHANGE CURRENT BRANCH
          // Push current state onto stack
          state.push_back(state.back());
          // rotate through color queue
          state.back().color = colorQueue.next();
          // state.back().color = al::Color(0, 1, 0);
      } else if (c == ']') {  // RESTORE PREVIOUS BRANCH
          // Pop previous state from stack
          state.pop_back();
          // state.back().color = al::Color(0, 0, 1);
      }
    }
    // --------------------------------------------------------------
    // 3. RENDER THE MAIN L-SYSTEM VERTICES
    // --------------------------------------------------------------
    // buildBranch(mainSystemMesh, pos, colors, Vec3f(0, 0, 0), RGB(1, 1, 1));
    mainSystemMesh.compress();

    // Set up the synth
    scene.registerSynthClass<SimpleVoice>();
    registerDynamicScene(scene);
    scene.verbose(true);  // turns on messages about voices to the console

    nav().pos(0, 0, 10); 
  }

  bool stepOn = false;
    bool onKeyDown(Keyboard const &k) {
        if (k.key() == ' ') {  // Start a new sequence through tree
          // new step state
          stepStates.push_back(StepState(pos, 0.05*(pos[0] - pos[1]).mag()));
          // stepOn = !stepOn;
          tonic = 220.0;
        }
        return true;
  }
  
  float angle = 0;
  float time = 0.f;
  // float interval = 1.f;
  int index = 0;
  void onAnimate(double dt) override {
    scene.update(dt);
    if (stepStates.size() > 0) {
      for (auto &state : stepStates) {
        // update current position
        state.elapsedTime += dt;
        if (state.elapsedTime > state.timeStep) {
          // reset internal clock
          state.elapsedTime = 0.f;

          // update current timestep
          state.timeStep = 0.05 * (state.path[index] - state.path[index + 1]).mag();
          
          // make sound at vertex
          auto *freeVoice = scene.getVoice<SimpleVoice>();
          Pose pose;
          pose.vec(mainSystemMesh.vertices().at(index));

          freeVoice->setPose(pose);
          scene.triggerOn(freeVoice);

          ++index;
          if (index >= mainSystemMesh.vertices().size()) {
            stepOn = false;
            // mainSystemMesh.vertices().erase(mainSystemMesh.vertices().begin() + index);
            index = 0;
            stepStates.pop_back();
          }
        }
      }
    }
    // nav().faceToward(Vec3d(0, 0, 0));
  }

  void onSound(AudioIOData &io) { scene.render(io); }

  void onDraw(Graphics &g) override {
    g.clear(0);
    g.meshColor();

    // g.rotate(angle, 0, 1, 0);
    // axes.draw(g);

    g.draw(mainSystemMesh);
    // g.draw(branchMesh);
    // mainSystemMesh.compress();
    // for (auto& ind : mainSystemMesh.vertices()) {
    //   std::cout << ind << std::endl;
    // }
    // --------------------------------------------------------------
    // 3. GENERATE AND RENDER BRANCHES
    // --------------------------------------------------------------
//     int nLayers = 1;  // how many layers of branches to generate
//     mainSystemMesh.compress();  // compress the mesh to remove redundant vertices
//     auto roots = mainSystemMesh.vertices();
//     std::vector<LSystemType> branchTypes = {LSystemType::BOURKE_BUSH_2};  // TODO: add more types

//     // Overlay `nLayers` of branches from the main L-system's vertices
//     for (int i = 0; i < nLayers; ++i) {
//         // randomly select from a list of LSystem types
//         LSystemType lsysType = branchTypes[rnd::uniformi(branchTypes.size())];
//         LSystem lsys = TYPE_DEF[lsysType];  // get the LSystem struct for the selected type
//         lsys.setRenderActions(&STD_RULES_DRAW);

//         float probability = 0.667;  // hardcoding for now
//         // min/max number of iterations for the branch L-system
//         std::pair iterRange = {1, N * 3};  // hardcoding for now
//         generateAndRenderBranches(roots, lsys, probability, iterRange);
//     }
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
