#include "l-system-template.hpp"

// L-system types ("Presets")
enum class LSystemType {
    BOURKE_BUSH_2,
    BOURKE_ALGAE_2,
    BOURKE_WEED,
    BOURKE_CRYSTAL,
    BOURKE_LEAF,
    ALGAE,
};

const std::map<LSystemType, std::string> TYPE_NAMES {
    {LSystemType::BOURKE_BUSH_2, "BOURKE_BUSH_2"},
    {LSystemType::BOURKE_ALGAE_2, "BOURKE_ALGAE_2"},
    {LSystemType::BOURKE_WEED, "BOURKE_WEED"},
    {LSystemType::BOURKE_CRYSTAL, "BOURKE_CRYSTAL"},
    {LSystemType::BOURKE_LEAF, "BOURKE_LEAF"},
    {LSystemType::ALGAE, "ALGAE"}
};

// Definitions for L-System types
const std::map<LSystemType, LSystem> TYPE_DEFS {
    {LSystemType::ALGAE, LSystem(
        {'F', 'f'},  // VARIABLES
        {},  // CONSTANTS
        "F",  // AXIOM
        33.33f,  // ANGLE
        1.0f,  // LENGTH
        1.667f,  // SCALE FACTOR
        {
            {'F', "-fF@"},
            {'f', "F+F+"}
        })  // RULES
    },

    {LSystemType::BOURKE_BUSH_2, LSystem(
        {'F'},  // VARIABLES
        {'[', ']', '+', '-'},  // CONSTANTS
        "F",  // AXIOM
        22.5f,  // ANGLE
        1.0f,  // LENGTH
        1.0f,  // SCALE FACTOR
        {{'F', "FF+[+F-F-F]-[-F+F+F]"}})  // RULES
    },

    {LSystemType::BOURKE_ALGAE_2, LSystem(
        {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y'},  // VARIABLES
        {'[', ']', '+', '-'},  // CONSTANTS
        "aF",  // AXIOM
        12.0f,  // ANGLE
        1.0f,  // LENGTH
        1.0f,  // SCALE FACTOR
        {
            {'a', "FFFFFy[++++n][----t]fb"},
            {'b', "+FFFFFy[++++n][----t]fc"},
            {'c', "FFFFFy[++++n][----t]fd"},
            {'d', "-FFFFFy[++++n][----t]fe"},
            {'e', "FFFFFy[++++n][----t]fg"},
            {'g', "FFFFFy[+++fa]fh"},
            {'h', "FFFFFy[++++n][----t]fi"},
            {'i', "+FFFFFy[++++n][----t]fj"},
            {'j', "FFFFFy[++++n][----t]fk"},
            {'k', "-FFFFFy[++++n][----t]fl"},
            {'l', "FFFFFy[++++n][----t]fm"},
            {'m', "FFFFFy[---fa]fa"},
            {'n', "ofFFF"},
            {'o', "fFFFp"},
            {'p', "fFFF[-s]q"},
            {'q', "fFFF[-s]r"},
            {'r', "fFFF[-s]"},
            {'s', "fFfF"},
            {'t', "ufFFF"},
            {'u', "fFFFv"},
            {'v', "fFFF[+s]w"},
            {'w', "fFFF[+s]x"},
            {'x', "fFFF[+s]"},
            {'y', "Fy"}
        })
    },

    {LSystemType::BOURKE_WEED, LSystem(
        {'F', 'X', 'Y'},  // VARIABLES
        {'[', ']', '+', '-'},  // CONSTANTS
        "F",  // AXIOM
        22.5f,  // ANGLE
        1.0f,  // LENGTH
        1.0f,  // SCALE FACTOR
        {
            {'F', "FF-[XY]+[XY]"},
            {'X', "+FY"},
            {'Y', "-FX"}
        })
    },

    {LSystemType::BOURKE_CRYSTAL, LSystem(
        {'F', 'X', 'Y'},  // VARIABLES
        {'+', '-'},  // CONSTANTS
        "F+F+F+F",  // AXIOM
        90.0f,  // ANGLE
        1.0f,  // LENGTH
        1.0f,  // SCALE FACTOR
        {
            {'F', "FF+F++F+F"}
        })
    },

    // axiom = a
    // F -> >F<
    // a -> F[+x]Fb
    // b -> F[-y]Fa
    // x -> a
    // y -> b
    // angle = 45
    // length factor = 1.36
    {LSystemType::BOURKE_LEAF, LSystem(
        {'F', 'a', 'b', 'x', 'y'},  // VARIABLES
        {'[', ']', '+', '-'},  // CONSTANTS
        "a",  // AXIOM
        45.0f,  // ANGLE
        1.0f,  // LENGTH
        1.36f,  // SCALE FACTOR
        {
            {'F', ">F<"},
            {'a', "F[+x]Fb"},
            {'b', "F[-y]Fa"},
            {'x', "a"},
            {'y', "b"}
        })
    }
};

