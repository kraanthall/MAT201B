// Karl Yerkes
// 2023-01-13
// MAT 201B
// Starter Code for Alssignment 1
//

#include "al/app/al_App.hpp"
#include "al/graphics/al_Image.hpp"
#include "al/io/al_File.hpp"
#include "al/math/al_Random.hpp"

using namespace al;
using namespace std;

struct AnApp : App {
  Mesh original, current, target;
  // hint: add more meshes here: cube, cylindar, custom
  Mesh rgb, hsv, extra;

  void onCreate() override {
    // note: File::currentPath() is going to be something like:
    //   /Users/foo/allolib_playground/code-folder/bin
    // and we will put an image file next to our .cpp in this folder
    //   /Users/foo/allolib_playground/code-folder
    // so we add the "../" to our image file name
    const std::string filename = File::currentPath() + "../mpv-shot0005.jpg";
    cout << "loading: " << filename << endl;
    auto image = Image(filename);
    if (image.array().size() == 0) {
      cout << "failed to load image " << filename << endl;
      exit(1);
    }
    cout << "image size: " << image.width() << "x" << image.height() << endl;

    Vec3d mean;  // note: we will learn average pixel position

    // get the RGB of each pixel in the image
    for (int column = 0; column < image.width(); ++column) {
      for (int row = 0; row < image.height(); ++row) {
        // here's how to look up a pixel in an image
        auto pixel = image.at(column, row);

        // pixel has .r .g .b and .a ~ each is an 8-bit unsigned integer
        // we need a float on (0, 1) so we divide by 255.0
        Color color(pixel.r / 255.0, pixel.g / 255.0, pixel.b / 255.0);

        // here we normalize the row and column while maintaining the
        // correct aspect ratio by dividing by width
        Vec3d position(1.0 * column / image.width(),
                       1.0 - 1.0 * row / image.width(), 0.0);
        mean += position;  // we will learn the average pixel position

        // add data to a mesh
        original.vertex(position);
        original.color(color);

        // add data to a mesh
        current.vertex(position);
        current.color(color);

        // hint: configure more meshes here

        // Target mesh
        target.vertex(position);
        target.color(color);

        // RGB color cube
        // rgb.vertex(position[0] / 255.0, position[1] / 255.0, position[2] / 255.0);
        // rgb.color(pixel.r / 255.0, pixel.g / 255.0, pixel.b / 255.0);

        // note: I had to look at the repo to figure out this fix
        rgb.vertex(color.r - 0.5, color.g - 0.5, color.b - 0.5);
        rgb.color(color);

        // TODO: HSV color cylinder
        // hsv.vertex( ... );
        // hsv.color( ... );

        // extra mesh
        // add a random double between -1 and 1 to each position
        extra.vertex(position[0] + rnd::uniformS(), position[1] + rnd::uniformS(), position[2] + rnd::uniformS());  
        // add a random double value to each color
        extra.color(color.r + rnd::uniformS(), color.g + rnd::uniformS(), color.b + rnd::uniformS());
      }
    }

    // here we center the image by subtracting the average position
    mean /= original.vertices().size();
    for (auto& v : original.vertices()) v -= mean;
    for (auto& v : current.vertices()) v -= mean;

    // configure the meshs to render as points
    original.primitive(Mesh::POINTS);
    current.primitive(Mesh::POINTS);
    target.primitive(Mesh::POINTS);

    // hint: configure the new meshes here
    rgb.primitive(Mesh::POINTS);
    hsv.primitive(Mesh::POINTS);
    extra.primitive(Mesh::POINTS);

    // set the viewer position 3 units back
    nav().pos(0, 0, 3);
  }

  double time = 1; // note: Sabina helped me with this
  void onAnimate(double dt) override {
    // hint: create an animation timer
    time += dt;

    // note: meshname.vertices() is the array of vertices
    // note: meshname.vertices().size() is the number of vertices
    // note: meshname.vertices()[5] is the vertex (a Vec3d) at index 5
    // hint: consider Vec3d::lerp(...) function

    // animate changes to the CURRENT mesh using linear interpolation
    if (time < 1) {
      int len = original.vertices().size();
      for (int i = 0; i < len; ++i) { current.vertices()[i].lerp(target.vertices()[i], time); }
    } else {
      current.vertices() = target.vertices();
    }
  }

  bool onKeyDown(Keyboard const& k) override {
    time = 0;
    switch (k.key()) {
      case '1': {
        // note: trigger transition back to original vertex positions
        target.vertices() = original.vertices();
      } break;
      case '2': {
        // note: trigger transition toward an RGB cube
        target.vertices() = rgb.vertices();
      } break;
      case '3': {
        // note: trigger transition toward an HSV cylinder
        target.vertices() = hsv.vertices();
      } break;
      case '4': {
        // note: trigger transition to your custom arrangement
        target.vertices() = extra.vertices();
      } break;
    }
    return true;
  }

  void onDraw(Graphics& g) override {
    // note: you don't need to touch anything here
    g.clear(0.25);
    g.meshColor();
    g.draw(current);  // draw the current mesh
  }
};

int main() {
  AnApp app;
  app.start();
  return 0;
}