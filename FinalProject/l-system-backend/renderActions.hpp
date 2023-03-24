#include <map>
#include <string>
// -------------------------------------------------------------------------------------------
// --------- "Standard" L-System Rule Sets ---------------------------------------------------
// 
// NOTE:  can the instruction strings be converted to switch cases or lambda functions which
// could then be used by `parser.hpp`?

// The "standard" L-system draw rules (if no rules are specified)
// Adapted from:  http://paulbourke.net/fractals/lsys/
const std::map<char, std::string> STD_RULES_DRAW {
    {'F', "Move forward by `LSystem.length` drawing a line"},
    {'f', "Move forward by `LSystem.length` without drawing a line"},
    {'+', "Turn left by `LSystem.angle`"},
    {'-', "Turn right by `LSystem.angle`"},
    {'|', "Reverse direction (ie: turn by 180 degrees)"},
    {'[', "Push current drawing state onto stack"},
    {']', "Pop current drawing state from the stack"},
    // {'#', "Increment the line width by line width increment"},
    // {'!', "Decrement the line width by line width increment"},
    {'@', "Draw a dot with line width radius"},
    // {'{', "Open a polygon"},
    // {'}', "Close a polygon and fill it with fill colour"},
    {'>', "Multiply the line length by the line length scale factor"},
    {'<', "Divide the line length by the line length scale factor"},
    {'&', "Swap the meaning of + and -"},  // negate the value of `LSystem.angle`(?)
    {'(', "Decrement turning angle by turning angle increment"},
    {')', "Increment turning angle by turning angle increment"}
};

// The "standard" L-system sound rules (if no rules are specified)
// FM-based sound action rules
const std::map<char, std::string> STD_RULES_FM {
    {'F', "Make sound of duration `LSystem.length`"},
    {'f', "Make rest of duration `LSystem.length`"},
    {'+', "Increase FM modulator/carrier ratio by `LSystem.angle`"},
    {'-', "Decrease FM modulator/carrier ratio by `LSystem.angle`"},
    {'|', "Invert FM modulator/carrier ratio"},
    {'[', "Push current state onto stack"},
    {']', "Pop current state from the stack"},
    {'#', "Increment the number of sidebands by 1"},
    {'!', "Decrement the number of sidebands by 1"},
    // {'@', NULL},
    // {'{', NULL},
    // {'}', NULL},
    {'>', "Multiply the duration length by `LSystem.scaleFactor`"},
    {'<', "Divide the duration length by `LSystem.scaleFactor`"},
    {'&', "Swap the meaning of + and -"},  // negate the value of `LSystem.angle`(?)
    {'(', "Decrement `LSystem.angle` by some epsilon"},
    {')', "Increment `LSystem.angle` by some epsilon"}
};

// -------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------