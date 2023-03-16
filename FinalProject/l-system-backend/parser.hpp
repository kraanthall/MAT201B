#include "l-system-types.hpp"
#include "renderActions.hpp"

#include "al/app/al_App.hpp"
#include "al/app/al_GUIDomain.hpp"
#include "al/math/al_Random.hpp"

using namespace al;
using namespace std;

#include <utility>
 
// NOTE:  this only generates the string, it does not draw/render anything.
// Render actions (e.g. draw a line, make a sound, etc.) are handled by a separate
// string parser that interprets the L-System string as a sequence of actions.
std::string generateString(LSystem lsys, int n) {
    std::string result = "";
    std::string current = lsys.axiom;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < current.length(); j++) {
            char c = current[j];
            if (lsys.rules.find(c) != lsys.rules.end()) {
                result += lsys.rules[c];
            } else {
                result += c;
            }
        }
        current = result;
        result = "";
    }
    return current;
}

void buildLine(Mesh &mesh, const Vec3f &start, const Vec3f &end, const RGB &startColor, const RGB &nextColor) {
    mesh.vertex(start);
    mesh.color(startColor);
    mesh.vertex(end);
    mesh.color(nextColor);
}

void buildBranch(Mesh &mesh, const std::vector<Vec3f> &pVec, const std::vector<RGB> &cVec, const Vec3f &startPoint, const RGB &startColor) {
    Vec3f currentPoint = startPoint;
    Color currentColor = startColor;

    for (int i = 0; i < pVec.size(); i++) {
        Vec3f nextPoint = currentPoint + pVec[i];
        Color nextColor = cVec[i];
        buildLine(mesh, currentPoint, nextPoint, currentColor, nextColor);
        currentPoint = nextPoint;
        currentColor = nextColor;
    }
}

// VISUAL RENDERING
void renderLSystem(const LSystem &lsys, const std::string &str, const al::Vec3f &startPoint, al::Mesh &mesh) {

// float decayRate = lsys.scaleFactor;

struct State : al::Pose {
    al::Color color;

    State(const al::Vec3f& position, const al::Color& color)
        : al::Pose(position), color(color)
    {}
};

std::vector<State> state;  // push_back / pop_back
// std::vector<al::Pose> state;  // push_back / pop_back

state.push_back(State{startPoint, al::Color(1, 1, 1)});
// state.push_back(al::Pose(startPoint));
// cout << "origin: " << state.back().pos() << endl;
// al::Pose& p(state.back());

// start the l-system at its origin
mesh.vertex(state.back().pos());
// mesh.color(state.back().color);
mesh.color(al::Color(1, 1, 1));

// stack.back(); // top
// stack.pop_back(); // pop
// stack.push_back(stack.back()); // push a copy of top

// mesh.vertices(); // ...is the list of redundant vertices
// mesh.compress(); // vertices + indices represetnation
// mesh.vertices(); // ...is the list of unique vertices
// mesh.indices();  // ...is the list of connectedness of vertices

for (char c : str) {
    if (c == 'F') {  // Move forward by `LSystem.length` drawing a line
        // ...
    } else if (c == '+') {
        
    } else if (c == '-') {

    } else if (c == '[') {  // CHANGE CURRENT BRANCH
        // Push current state onto stack
        state.push_back(state.back());
    } else if (c == ']') {  // RESTORE PREVIOUS BRANCH
        // Pop previous state from stack
        state.pop_back();
    }
}
// g.draw(mesh);
}

// AUDIO RENDERING
void renderLSystem(const LSystem &lsys, const std::string &str, float f) {


}
