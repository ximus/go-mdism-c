#include <sys/types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include <fcntl.h>
#include <sys/stat.h>

#ifdef DEBUG
#define DEBUG_PRINT(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( false )
#else
#define DEBUG_PRINT(...) do{ } while ( false )
#endif


/**
 * Model defnitions
 */

typedef struct {
  id_t id;
  id_t model_id;
  char *name;
  bool is_zero_state;
} state_t;

typedef struct {
  id_t id;
  char *name;
  state_t *zero_state;
} model_t;

typedef struct {
  id_t cycle_id;
  id_t person_id;
  id_t state_id;
  id_t model_id;
} master_record_t;

typedef struct {
  id_t id;
  char *name;
} cycle_t;

typedef struct {
  id_t id;
} person_t;

typedef struct {
  id_t id;
  id_t cur_state_id;
  id_t from_state_id;
  id_t to_state_id;
  float adjustment;
  id_t effected_model_id;
} interaction_t;

typedef struct {
  id_t id;
  id_t from_id;
  id_t to_id;
  float prob;
} trans_prob_t;

/* EOF model definitions */


/**
 * Constant initializations
 * all model `id` fields need match the instance's position in model array
 */

model_t models[] = {
  {0, "NAFLD"},
  {1, "CHD"},
  {2, "T2DM"},
  {3, "BMI"},
  {4, "Ethnicity"},
  {5, "Sex"},
  {6, "Physicial activity"},
  {7, "Fructose"},
  {8, "Trans Fat"},
  {9, "N3-PUFA"},
  {10, "Age"}
};

state_t states[] = {
  {0, 0, "Unitialized1", true},
  {1, 0, "Unitialized2", false},
  {2, 0, "No NAFLD", false},
  {3, 0, "Steatosis", false},
  {4, 0, "NASH", false},
  {5, 0, "Cirrhosis", false},
  {6, 0, "HCC", false},
  {7, 0, "Liver death", false},
  {8, 0, "Natural death", false},
  {9, 0, "Other death", false},
  {10, 1, "Unitialized1", true},
  {11, 1, "Unitialized2", false},
  {12, 1, "No CHD", false},
  {13, 1, "CHD", false},
  {14, 1, "CHD Death", false},
  {15, 1, "Other death", false},
  {16, 2, "Unitialized1", true},
  {17, 2, "Unitialized2", false},
  {18, 2, "No T2DM", false},
  {19, 2, "T2DM", false},
  {20, 2, "T2DM Death", false},
  {21, 2, "Other death", false},
  {22, 3, "Unitialized1", true},
  {23, 3, "Unitialized2", false},
  {24, 3, "Healthy weight", false},
  {25, 3, "Overweight", false},
  {26, 3, "Obese", false},
  {27, 3, "Other death", false},
  {28, 4, "Unitialized1", true},
  {29, 4, "Non-hispanic white", false},
  {30, 4, "Non-hispanic black", false},
  {31, 4, "Hispanic", false},
  {32, 4, "Other death", false},
  {33, 5, "Unitialized1", true},
  {34, 5, "Male", false},
  {35, 5, "Female", false},
  {36, 5, "Other death", false},
  {37, 6, "Unitialized1", true},
  {38, 6, "Unitialized2", false},
  {39, 6, "Active", false},
  {40, 6, "Inactive", false},
  {41, 6, "Other death", false},
  {42, 7, "Unitialized1", true},
  {43, 7, "Increase risk", false},
  {44, 7, "No increased risk", false},
  {45, 7, "Other death", false},
  {46, 8, "Unitialized1", true},
  {47, 8, "Increase risk", false},
  {48, 8, "No increased risk", false},
  {49, 8, "Other death", false},
  {50, 9, "Unitialized1", true},
  {51, 9, "Decreased risk", false},
  {52, 9, "No decreased risk", false},
  {53, 9, "Other death", false},
  {54, 10, "Unitialized1", true},
  {55, 10, "Age of 20", false},
  {56, 10, "Age of 21", false},
  {57, 10, "Age of 22", false},
  {58, 10, "Age of 23", false},
  {59, 10, "Age of 24", false},
  {60, 10, "Age of 25", false},
  {61, 10, "Age of 26", false},
  {62, 10, "Age of 27", false},
  {63, 10, "Age of 28", false},
  {64, 10, "Age of 29", false},
  {65, 10, "Age of 30", false},
  {66, 10, "Age of 31", false},
  {67, 10, "Age of 32", false},
  {68, 10, "Age of 33", false},
  {69, 10, "Age of 34", false},
  {70, 10, "Age of 35", false},
  {71, 10, "Age of 36", false},
  {72, 10, "Age of 37", false},
  {73, 10, "Age of 38", false},
  {74, 10, "Age of 39", false},
  {75, 10, "Age of 40", false},
  {76, 10, "Age of 41", false},
  {77, 10, "Age of 42", false},
  {78, 10, "Age of 43", false},
  {79, 10, "Age of 44", false},
  {80, 10, "Age of 45", false},
  {81, 10, "Age of 46", false},
  {82, 10, "Age of 47", false},
  {83, 10, "Age of 48", false},
  {84, 10, "Age of 49", false},
  {85, 10, "Age of 50", false},
  {86, 10, "Age of 51", false},
  {87, 10, "Age of 52", false},
  {88, 10, "Age of 53", false},
  {89, 10, "Age of 54", false},
  {90, 10, "Age of 55", false},
  {91, 10, "Age of 56", false},
  {92, 10, "Age of 57", false},
  {93, 10, "Age of 58", false},
  {94, 10, "Age of 59", false},
  {95, 10, "Age of 60", false},
  {96, 10, "Age of 61", false},
  {97, 10, "Age of 62", false},
  {98, 10, "Age of 63", false},
  {99, 10, "Age of 64", false},
  {100, 10, "Age of 65", false},
  {101, 10, "Age of 66", false},
  {102, 10, "Age of 67", false},
  {103, 10, "Age of 68", false},
  {104, 10, "Age of 69", false},
  {105, 10, "Age of 70", false},
  {106, 10, "Age of 71", false},
  {107, 10, "Age of 72", false},
  {108, 10, "Age of 73", false},
  {109, 10, "Age of 74", false},
  {110, 10, "Age of 75", false},
  {111, 10, "Age of 76", false},
  {112, 10, "Age of 77", false},
  {113, 10, "Age of 78", false},
  {114, 10, "Age of 79", false},
  {115, 10, "Age of 80", false},
  {116, 10, "Age of 81", false},
  {117, 10, "Age of 82", false},
  {118, 10, "Age of 83", false},
  {119, 10, "Age of 84", false},
  {120, 10, "Age of 85", false},
  {121, 10, "Age of 86", false},
  {122, 10, "Age of 87", false},
  {123, 10, "Age of 88", false},
  {124, 10, "Age of 89", false},
  {125, 10, "Age of 90", false},
  {126, 10, "Age of 91", false},
  {127, 10, "Age of 92", false},
  {128, 10, "Age of 93", false},
  {129, 10, "Age of 94", false},
  {130, 10, "Age of 95", false},
  {131, 10, "Age of 96", false},
  {132, 10, "Age of 97", false},
  {133, 10, "Age of 98", false},
  {134, 10, "Age of 99", false},
  {135, 10, "Age of 100", false},
  {136, 10, "Age of 101", false},
  {137, 10, "Age of 102", false},
  {138, 10, "Age of 103", false},
  {139, 10, "Age of 104", false},
  {140, 10, "Age of 105", false},
  {141, 10, "Age of 106", false},
  {142, 10, "Age of 107", false},
  {143, 10, "Age of 108", false},
  {144, 10, "Age of 109", false},
  {145, 10, "Age of 110", false}
};

trans_prob_t trans_probs[] = {
  {0, 0, 0, 0},
  {1, 0, 1, 1},
  {2, 0, 2, 0},
  {3, 0, 3, 0},
  {4, 0, 4, 0},
  {5, 0, 5, 0},
  {6, 0, 6, 0},
  {7, 0, 7, 0},
  {8, 0, 8, 0},
  {9, 0, 9, 0},
  {10, 1, 0, 0},
  {11, 1, 1, 0},
  {12, 1, 2, 0.7},
  {13, 1, 3, 0.234},
  {14, 1, 4, 0.06},
  {15, 1, 5, 0.006},
  {16, 1, 6, 0},
  {17, 1, 7, 0},
  {18, 1, 8, 0},
  {19, 1, 9, 0},
  {20, 2, 0, 0},
  {21, 2, 1, 0},
  {22, 2, 2, 0.9797},
  {23, 2, 3, 0.01},
  {24, 2, 4, 0.0003},
  {25, 2, 5, 0},
  {26, 2, 6, 0},
  {27, 2, 7, 0},
  {28, 2, 8, 0.01},
  {29, 2, 9, 0},
  {30, 3, 0, 0},
  {31, 3, 1, 0},
  {32, 3, 2, 0.02},
  {33, 3, 3, 0.9638},
  {34, 3, 4, 0.006},
  {35, 3, 5, 0.0002},
  {36, 3, 6, 0},
  {37, 3, 7, 0},
  {38, 3, 8, 0.01},
  {39, 3, 9, 0},
  {40, 4, 0, 0},
  {41, 4, 1, 0},
  {42, 4, 2, 0.001},
  {43, 4, 3, 0.02},
  {44, 4, 4, 0.9669},
  {45, 4, 5, 0.002},
  {46, 4, 6, 0.0001},
  {47, 4, 7, 0},
  {48, 4, 8, 0.01},
  {49, 4, 9, 0},
  {50, 5, 0, 0},
  {51, 5, 1, 0},
  {52, 5, 2, 0},
  {53, 5, 3, 0},
  {54, 5, 4, 0},
  {55, 5, 5, 0.97},
  {56, 5, 6, 0.02},
  {57, 5, 7, 0},
  {58, 5, 8, 0.01},
  {59, 5, 9, 0},
  {60, 6, 0, 0},
  {61, 6, 1, 0},
  {62, 6, 2, 0},
  {63, 6, 3, 0},
  {64, 6, 4, 0},
  {65, 6, 5, 0},
  {66, 6, 6, 0.5},
  {67, 6, 7, 0.5},
  {68, 6, 8, 0},
  {69, 6, 9, 0},
  {70, 7, 0, 0},
  {71, 7, 1, 0},
  {72, 7, 2, 0},
  {73, 7, 3, 0},
  {74, 7, 4, 0},
  {75, 7, 5, 0},
  {76, 7, 6, 0},
  {77, 7, 7, 1},
  {78, 7, 8, 0},
  {79, 7, 9, 0},
  {80, 8, 0, 0},
  {81, 8, 1, 0},
  {82, 8, 2, 0},
  {83, 8, 3, 0},
  {84, 8, 4, 0},
  {85, 8, 5, 0},
  {86, 8, 6, 0},
  {87, 8, 7, 0},
  {88, 8, 8, 1},
  {89, 8, 9, 0},
  {90, 9, 0, 0},
  {91, 9, 1, 0},
  {92, 9, 2, 0},
  {93, 9, 3, 0},
  {94, 9, 4, 0},
  {95, 9, 5, 0},
  {96, 9, 6, 0},
  {97, 9, 7, 0},
  {98, 9, 8, 0},
  {99, 9, 9, 1},
  {100, 10, 10, 0},
  {101, 10, 11, 1},
  {102, 10, 12, 0},
  {103, 10, 13, 0},
  {104, 10, 14, 0},
  {105, 10, 15, 0},
  {106, 11, 10, 0},
  {107, 11, 11, 0},
  {108, 11, 12, 0.95},
  {109, 11, 13, 0.05},
  {110, 11, 14, 0},
  {111, 11, 15, 0},
  {112, 12, 10, 0},
  {113, 12, 11, 0},
  {114, 12, 12, 0.995},
  {115, 12, 13, 0.005},
  {116, 12, 14, 0},
  {117, 12, 15, 0},
  {118, 13, 10, 0},
  {119, 13, 11, 0},
  {120, 13, 12, 0},
  {121, 13, 13, 0.99},
  {122, 13, 14, 0.01},
  {123, 13, 15, 0},
  {124, 14, 10, 0},
  {125, 14, 11, 0},
  {126, 14, 12, 0},
  {127, 14, 13, 0},
  {128, 14, 14, 1},
  {129, 14, 15, 0},
  {130, 15, 10, 0},
  {131, 15, 11, 0},
  {132, 15, 12, 0},
  {133, 15, 13, 0},
  {134, 15, 14, 0},
  {135, 15, 15, 1},
  {136, 16, 16, 0},
  {137, 16, 17, 1},
  {138, 16, 18, 0},
  {139, 16, 19, 0},
  {140, 16, 20, 0},
  {141, 16, 21, 0},
  {142, 17, 16, 0},
  {143, 17, 17, 0},
  {144, 17, 18, 0.9},
  {145, 17, 19, 0.1},
  {146, 17, 20, 0},
  {147, 17, 21, 0},
  {148, 18, 16, 0},
  {149, 18, 17, 0},
  {150, 18, 18, 0.99},
  {151, 18, 19, 0.01},
  {152, 18, 20, 0},
  {153, 18, 21, 0},
  {154, 19, 16, 0},
  {155, 19, 17, 0},
  {156, 19, 18, 0},
  {157, 19, 19, 0.99},
  {158, 19, 20, 0.01},
  {159, 19, 21, 0},
  {160, 20, 16, 0},
  {161, 20, 17, 0},
  {162, 20, 18, 0},
  {163, 20, 19, 0},
  {164, 20, 20, 1},
  {165, 20, 21, 0},
  {166, 21, 16, 0},
  {167, 21, 17, 0},
  {168, 21, 18, 0},
  {169, 21, 19, 0},
  {170, 21, 20, 0},
  {171, 21, 21, 1},
  {172, 22, 22, 0},
  {173, 22, 23, 1},
  {174, 22, 24, 0},
  {175, 22, 25, 0},
  {176, 22, 26, 0},
  {177, 22, 27, 0},
  {178, 23, 22, 0},
  {179, 23, 23, 0},
  {180, 23, 24, 0.35},
  {181, 23, 25, 0.35},
  {182, 23, 26, 0.3},
  {183, 23, 27, 0},
  {184, 24, 22, 0},
  {185, 24, 23, 0},
  {186, 24, 24, 0.944},
  {187, 24, 25, 0.05},
  {188, 24, 26, 0.006},
  {189, 24, 27, 0},
  {190, 25, 22, 0},
  {191, 25, 23, 0},
  {192, 25, 24, 0.03},
  {193, 25, 25, 0.946},
  {194, 25, 26, 0.024},
  {195, 25, 27, 0},
  {196, 26, 22, 0},
  {197, 26, 23, 0},
  {198, 26, 24, 0.002},
  {199, 26, 25, 0.02},
  {200, 26, 26, 0.978},
  {201, 26, 27, 0},
  {202, 27, 22, 0},
  {203, 27, 23, 0},
  {204, 27, 24, 0},
  {205, 27, 25, 0},
  {206, 27, 26, 0},
  {207, 27, 27, 1},
  {208, 28, 28, 0},
  {209, 28, 29, 0.743771},
  {210, 28, 30, 0.115852},
  {211, 28, 31, 0.140377},
  {212, 28, 32, 0},
  {213, 29, 28, 0},
  {214, 29, 29, 1},
  {215, 29, 30, 0},
  {216, 29, 31, 0},
  {217, 29, 32, 0},
  {218, 30, 28, 0},
  {219, 30, 29, 0},
  {220, 30, 30, 1},
  {221, 30, 31, 0},
  {222, 30, 32, 0},
  {223, 31, 28, 0},
  {224, 31, 29, 0},
  {225, 31, 30, 0},
  {226, 31, 31, 1},
  {227, 31, 32, 0},
  {228, 32, 28, 0},
  {229, 32, 29, 0},
  {230, 32, 30, 0},
  {231, 32, 31, 0},
  {232, 32, 32, 1},
  {233, 33, 33, 0},
  {234, 33, 34, 0.484388},
  {235, 33, 35, 0.515612},
  {236, 33, 36, 0},
  {237, 34, 33, 0},
  {238, 34, 34, 1},
  {239, 34, 35, 0},
  {240, 34, 36, 0},
  {241, 35, 33, 0},
  {242, 35, 34, 0},
  {243, 35, 35, 1},
  {244, 35, 36, 0},
  {245, 36, 33, 0},
  {246, 36, 34, 0},
  {247, 36, 35, 0},
  {248, 36, 36, 1},
  {249, 37, 37, 0},
  {250, 37, 38, 1},
  {251, 37, 39, 0},
  {252, 37, 40, 0},
  {253, 37, 41, 0},
  {254, 38, 37, 0},
  {255, 38, 38, 0},
  {256, 38, 39, 0.3},
  {257, 38, 40, 0.7},
  {258, 38, 41, 0},
  {259, 39, 37, 0},
  {260, 39, 38, 0},
  {261, 39, 39, 0.9},
  {262, 39, 40, 0.1},
  {263, 39, 41, 0},
  {264, 40, 37, 0},
  {265, 40, 38, 0},
  {266, 40, 39, 0.1},
  {267, 40, 40, 0.9},
  {268, 40, 41, 0},
  {269, 41, 37, 0},
  {270, 41, 38, 0},
  {271, 41, 39, 0},
  {272, 41, 40, 0},
  {273, 41, 41, 1},
  {274, 42, 42, 0},
  {275, 42, 43, 0.70004767},
  {276, 42, 44, 0.29995233},
  {277, 42, 45, 0},
  {278, 43, 42, 0},
  {279, 43, 43, 0.95},
  {280, 43, 44, 0.05},
  {281, 43, 45, 0},
  {282, 44, 42, 0},
  {283, 44, 43, 0.05},
  {284, 44, 44, 0.95},
  {285, 44, 45, 0},
  {286, 45, 42, 0},
  {287, 45, 43, 0},
  {288, 45, 44, 0},
  {289, 45, 45, 1},
  {290, 46, 46, 0},
  {291, 46, 47, 0.023},
  {292, 46, 48, 0.977},
  {293, 46, 49, 0},
  {294, 47, 46, 0},
  {295, 47, 47, 1},
  {296, 47, 48, 0},
  {297, 47, 49, 0},
  {298, 48, 46, 0},
  {299, 48, 47, 0},
  {300, 48, 48, 1},
  {301, 48, 49, 0},
  {302, 49, 46, 0},
  {303, 49, 47, 0},
  {304, 49, 48, 0},
  {305, 49, 49, 1},
  {306, 50, 50, 0},
  {307, 50, 51, 0.6306},
  {308, 50, 52, 0.3694},
  {309, 50, 53, 0},
  {310, 51, 50, 0},
  {311, 51, 51, 1},
  {312, 51, 52, 0},
  {313, 51, 53, 0},
  {314, 52, 50, 0},
  {315, 52, 51, 0},
  {316, 52, 52, 1},
  {317, 52, 53, 0},
  {318, 53, 50, 0},
  {319, 53, 51, 0},
  {320, 53, 52, 0},
  {321, 53, 53, 1},
  {322, 54, 55, 0.019194468},
  {323, 54, 56, 0.019194468},
  {324, 54, 57, 0.019194468},
  {325, 54, 58, 0.019194468},
  {326, 54, 59, 0.019194468},
  {327, 54, 60, 0.018700907},
  {328, 54, 61, 0.018700907},
  {329, 54, 62, 0.018700907},
  {330, 54, 63, 0.018700907},
  {331, 54, 64, 0.018700907},
  {332, 54, 65, 0.0177493},
  {333, 54, 66, 0.0177493},
  {334, 54, 67, 0.0177493},
  {335, 54, 68, 0.0177493},
  {336, 54, 69, 0.0177493},
  {337, 54, 70, 0.017756917},
  {338, 54, 71, 0.017756917},
  {339, 54, 72, 0.017756917},
  {340, 54, 73, 0.017756917},
  {341, 54, 74, 0.017756917},
  {342, 54, 75, 0.018486967},
  {343, 54, 76, 0.018486967},
  {344, 54, 77, 0.018486967},
  {345, 54, 78, 0.018486967},
  {346, 54, 79, 0.018486967},
  {347, 54, 80, 0.020017816},
  {348, 54, 81, 0.020017816},
  {349, 54, 82, 0.020017816},
  {350, 54, 83, 0.020017816},
  {351, 54, 84, 0.020017816},
  {352, 54, 85, 0.019766711},
  {353, 54, 86, 0.019766711},
  {354, 54, 87, 0.019766711},
  {355, 54, 88, 0.019766711},
  {356, 54, 89, 0.019766711},
  {357, 54, 90, 0.017505182},
  {358, 54, 91, 0.017505182},
  {359, 54, 92, 0.017505182},
  {360, 54, 93, 0.017505182},
  {361, 54, 94, 0.017505182},
  {362, 54, 95, 0.015024017},
  {363, 54, 96, 0.015024017},
  {364, 54, 97, 0.015024017},
  {365, 54, 98, 0.015024017},
  {366, 54, 99, 0.015024017},
  {367, 54, 100, 0.011072776},
  {368, 54, 101, 0.011072776},
  {369, 54, 102, 0.011072776},
  {370, 54, 103, 0.011072776},
  {371, 54, 104, 0.011072776},
  {372, 54, 105, 0.008256161},
  {373, 54, 106, 0.008256161},
  {374, 54, 107, 0.008256161},
  {375, 54, 108, 0.008256161},
  {376, 54, 109, 0.008256161},
  {377, 54, 110, 0.006472549},
  {378, 54, 111, 0.006472549},
  {379, 54, 112, 0.006472549},
  {380, 54, 113, 0.006472549},
  {381, 54, 114, 0.006472549},
  {382, 54, 115, 0.005092902},
  {383, 54, 116, 0.005092902},
  {384, 54, 117, 0.005092902},
  {385, 54, 118, 0.005092902},
  {386, 54, 119, 0.005092902},
  {387, 54, 120, 0.024516635},
  {388, 55, 56, 1},
  {389, 56, 57, 1},
  {390, 57, 58, 1},
  {391, 58, 59, 1},
  {392, 59, 60, 1},
  {393, 60, 61, 1},
  {394, 61, 62, 1},
  {395, 62, 63, 1},
  {396, 63, 64, 1},
  {397, 64, 65, 1},
  {398, 65, 66, 1},
  {399, 66, 67, 1},
  {400, 67, 68, 1},
  {401, 68, 69, 1},
  {402, 69, 70, 1},
  {403, 70, 71, 1},
  {404, 71, 72, 1},
  {405, 72, 73, 1},
  {406, 73, 74, 1},
  {407, 74, 75, 1},
  {408, 75, 76, 1},
  {409, 76, 77, 1},
  {410, 77, 78, 1},
  {411, 78, 79, 1},
  {412, 79, 80, 1},
  {413, 80, 81, 1},
  {414, 81, 82, 1},
  {415, 82, 83, 1},
  {416, 83, 84, 1},
  {417, 84, 85, 1},
  {418, 85, 86, 1},
  {419, 86, 87, 1},
  {420, 87, 88, 1},
  {421, 88, 89, 1},
  {422, 89, 90, 1},
  {423, 90, 91, 1},
  {424, 91, 92, 1},
  {425, 92, 93, 1},
  {426, 93, 94, 1},
  {427, 94, 95, 1},
  {428, 95, 96, 1},
  {429, 96, 97, 1},
  {430, 97, 98, 1},
  {431, 98, 99, 1},
  {432, 99, 100, 1},
  {433, 100, 101, 1},
  {434, 101, 102, 1},
  {435, 102, 103, 1},
  {436, 103, 104, 1},
  {437, 104, 105, 1},
  {438, 105, 106, 1},
  {439, 106, 107, 1},
  {440, 107, 108, 1},
  {441, 108, 109, 1},
  {442, 109, 110, 1},
  {443, 110, 111, 1},
  {444, 111, 112, 1},
  {445, 112, 113, 1},
  {446, 113, 114, 1},
  {447, 114, 115, 1},
  {448, 115, 116, 1},
  {449, 116, 117, 1},
  {450, 117, 118, 1},
  {451, 118, 119, 1},
  {452, 119, 120, 1},
  {453, 120, 121, 1},
  {454, 121, 122, 1},
  {455, 122, 123, 1},
  {456, 123, 124, 1},
  {457, 124, 125, 1},
  {458, 125, 126, 1},
  {459, 126, 127, 1},
  {460, 127, 128, 1},
  {461, 128, 129, 1},
  {462, 129, 130, 1},
  {463, 130, 131, 1},
  {464, 131, 132, 1},
  {465, 132, 133, 1},
  {466, 133, 134, 1},
  {467, 134, 135, 1},
  {468, 135, 136, 1},
  {469, 136, 137, 1},
  {470, 137, 138, 1},
  {471, 138, 139, 1},
  {472, 139, 140, 1},
  {473, 140, 141, 1},
  {474, 141, 142, 1},
  {475, 142, 143, 1},
  {476, 143, 144, 1},
  {477, 144, 145, 1},
  {478, 145, 145, 1}
};

interaction_t interactions[] = {
  {0, 25, 2, 3, 1.8, 0},
  {1, 26, 2, 3, 2.5, 0},
  {2, 30, 2, 3, 0.93, 0},
  {3, 31, 2, 3, 1.67, 0}
};

cycle_t cycles[] = {
  { 0,  "Pre-initialization" },
  { 1,  "2015" },
  { 2,  "2016" },
  { 3,  "2017" },
  { 4,  "2018" },
  { 5,  "2018" },
  { 6,  "2018" },
  { 7,  "2018" },
  { 8,  "2018" },
  { 9,  "2018" },
  { 10, "2018" },
  { 11, "2018" },
  { 12, "2018" },
  { 13, "2018" },
  { 14, "2018" },
  { 15, "2018" },
  { 16, "2018" },
  { 17, "2018" },
  { 18, "2018" },
  { 19, "2018" }
};

/* EOF constants */


// maybe should CAPITALIZE these consts
const size_t PERSON_COUNT = 1000;
const size_t STATE_COUNT = sizeof(states) / sizeof(state_t);
const size_t MODEL_COUNT = sizeof(models) / sizeof(model_t);
const size_t CYCLE_COUNT = sizeof(cycles) / sizeof(cycle_t);
const size_t TRANS_PROB_COUNT  = sizeof(trans_probs) / sizeof(trans_prob_t);
const size_t INTERACTION_COUNT = sizeof(interactions) / sizeof(interaction_t);

person_t *people;
master_record_t *masters;
uint master_count = 0;


/**
 * Indexes
 */

typedef struct {
    size_t x_size;
    size_t y_size;
    size_t z_size;
    id_t *data;
} index_3d_t;

uint index_3d_offset(index_3d_t *index, uint z, uint y, uint x)
{
    return ( z * index->x_size * index->y_size ) +
           ( y * index->x_size ) +
           x;
}

id_t index_3d_get(index_3d_t *index, uint z, uint y, uint x)
{
    uint offset = index_3d_offset(index, x, y, z);
    return index->data[offset];
}

id_t *index_3d_get_2d(index_3d_t *index, uint z, uint y)
{
    uint offset = ( z * index->x_size * index->y_size ) +
                  ( y * index->x_size );
    return &index->data[offset];
}

void index_3d_put(index_3d_t *index, uint z, uint y, uint x, id_t val)
{
    uint offset = index_3d_offset(index, x, y, z);
    index->data[offset] = val;
}

void index_3d_init(index_3d_t *index, size_t z, size_t y, size_t x)
{
    index->x_size = x;
    index->y_size = y;
    index->z_size = z;
    index->data = malloc(x * y * z * sizeof(*index->data));
}

index_3d_t state_id_by_cycle_person_model;
trans_prob_t **trans_probs_by_state[STATE_COUNT];
size_t state_get_dest_count(state_t *);

void init_trans_probs_by_state(void)
{
    /* for every state */
    for (int sid = 0; sid < STATE_COUNT; ++sid)
    {
        id_t model_id = states[sid].model_id;
        size_t dest_count = state_get_dest_count(&states[sid]);
        /* will store refs to all transitions from this state */
        trans_prob_t **value = malloc(dest_count * sizeof(trans_prob_t*));

        uint i = 0;
        /* for every transition probability */
        for (int tb_id = 0; tb_id < TRANS_PROB_COUNT; ++tb_id)
        {
            trans_prob_t *tb = &trans_probs[tb_id];
            if (tb->from_id == sid)
            {
                value[i++] = tb;
            }
        }
        trans_probs_by_state[sid] = value;
    }
}

void init_indexes(void)
{
    index_3d_init(&state_id_by_cycle_person_model, CYCLE_COUNT, PERSON_COUNT, MODEL_COUNT);
    init_trans_probs_by_state();
}

/* EOF indexes */


/**
 * Master records helpers
 */

void init_masters(void)
{
    size_t count = CYCLE_COUNT * MODEL_COUNT * STATE_COUNT * PERSON_COUNT;
    masters = malloc(count * sizeof(master_record_t));
}

void masters_push(id_t cycle_id, id_t state_id, id_t model_id, id_t person_id)
{
    DEBUG_PRINT("run_cycle(): cycle_id=%d person_id=%d model_id=%d state_id=%d\n", cycle_id, person_id, model_id, state_id);
    masters[master_count].cycle_id = cycle_id;
    masters[master_count].state_id = state_id;
    masters[master_count].model_id = model_id;
    masters[master_count].person_id = person_id;
    master_count += 1;
}


/**
 * Person helpers
 */

void person_add_state(id_t person_id, id_t cycle_id, state_t *state)
{
    index_3d_put(
        &state_id_by_cycle_person_model,
        cycle_id, person_id, state->model_id,
        state->id
    );
    masters_push(cycle_id, state->id, state->model_id, person_id);
}

state_t *person_get_state(id_t person_id, id_t cycle_id, id_t model_id)
{
    id_t id = index_3d_get(
        &state_id_by_cycle_person_model,
        cycle_id, person_id, model_id
    );
    return &states[id];
}

id_t *person_get_states(id_t person_id, id_t cycle_id)
{
    return index_3d_get_2d(
        &state_id_by_cycle_person_model,
        cycle_id, person_id
    );
}


/**
 * Model helpers
 */

 void init_models(void)
 {
     /* map model to model's zero state */
     /* maybe this should be considered an index and go to init_indexes() */
     for (int i = 0; i < STATE_COUNT; ++i)
     {
         if (states[i].is_zero_state)
         {
             models[states[i].model_id].zero_state = &states[i];
         }
     }
 }

size_t model_state_counts[MODEL_COUNT] = {0};

size_t model_get_state_count(id_t model_id)
{
    if (model_state_counts[model_id] == 0)
    {
        for (int sid = 0; sid < STATE_COUNT; ++sid)
        {
            if (states[sid].model_id == model_id)
            {
                model_state_counts[model_id] += 1;
            }
        }
    }
    return model_state_counts[model_id];
}


/**
 * State helpers
 */

size_t state_get_dest_count(state_t *state)
{
    return model_get_state_count(state->model_id);
}


/* also inits people */
void init_world(void)
{
    people = malloc(PERSON_COUNT * sizeof(person_t));

    /* for every person */
    for (int pid = 0; pid < PERSON_COUNT; ++pid)
    {
        people[pid].id = pid;
        /* for every model */
        for (int mid = 0; mid < MODEL_COUNT; ++mid)
        {
            /* get model's zero state */
            state_t *zero_state = models[mid].zero_state;
            /* assign zero state to person */
            person_add_state(pid, 0, zero_state);

        }
    }
}

// http://stackoverflow.com/questions/2509679/how-to-generate-a-random-number-from-within-a-range
uint rand_interval(uint min, uint max)
{
    int r;
    const uint range = 1 + max - min;
    const uint buckets = RAND_MAX / range;
    const uint limit = buckets * range;

    /* Create equal size buckets all in a row, then fire randomly towards
     * the buckets until you land in one of them. All buckets are equally
     * likely. If you land off the end of the line of buckets, try again. */
    do
    {
        r = rand();
    } while (r >= limit);

    return min + (r / buckets);
}

void suffle(uint *items, size_t len)
{
    // Knuth shuffle
    // http://stackoverflow.com/questions/14985737/how-to-access-a-c-array-in-random-order-ensuring-that-all-elements-are-visited-a
    for (int i = len-1; i > 1; --i)
    {
        uint j = rand_interval(0, i);
        uint itemi = items[i];
        items[i] = items[j];
        items[j] = itemi;
    }
}

/**
 * malloc the data structure that will be used to store probabilities
 * and destination state_ids during model runs. The `prob` field will
 * be adjusted if need by by interactions.
 * The size of this variable will be the count of the number of states
 * of the model with the most states.
 */
trans_prob_t *malloc_adjusted_probs(void)
{
    size_t max = 0;
    for (int mid = 0; mid < MODEL_COUNT; ++mid)
    {
        size_t count = model_get_state_count(mid);
        if (count > max)
        {
            max = count;
        }
    }
    return malloc(max * sizeof(trans_prob_t));
}

/**
 * @param adj_probs      array of transition probabilities that will be altered
 *                       during modelling
 * @param starting_probs array of transition probabilities that will be copied
 *                       to `adj_probs` as starting values.
 */
void init_adjusted_probs(trans_prob_t *adj_probs, trans_prob_t **starting_probs, size_t starting_probs_count)
{
    for (int i = 0; i < starting_probs_count; ++i)
    {
        adj_probs[i] = *starting_probs[i];
    }
}

state_t *pickState(trans_prob_t *tps, size_t tp_count)
{
    float sum = 0;
    float random = (float) rand() / (float) RAND_MAX;
    for (int i = 0; i < tp_count; ++i)
    {
        sum += tps[i].prob;
        if (random <= sum)
        {
            return &states[tps[i].to_id];
        }
    }
    return NULL; /* error */
}

const uint INTERACTIONS_MAX = MODEL_COUNT * MODEL_COUNT;

/* maybe namespace or encapsulate the global data storage objects
   to avoid accidental conflicts with local variables */
void run_cycle(id_t cycle_id, id_t person_id, id_t model_id)
{
    // DEBUG_PRINT("run_cycle(): cycle_id=%d person_id=%d model_id=%d\n", cycle_id, person_id, model_id);

    state_t *cur_state = person_get_state(person_id, cycle_id - 1, model_id);
    id_t *cur_state_ids = person_get_states(person_id, cycle_id - 1);
    /* always in one state for each model */
    size_t cur_states_count = MODEL_COUNT;

    trans_prob_t **cur_trans_probs = trans_probs_by_state[cur_state->id];
    size_t cur_trans_prob_count = state_get_dest_count(cur_state);

    static interaction_t *cur_interactions[INTERACTIONS_MAX];
    size_t cur_interaction_count = 0;

    static trans_prob_t *adjusted_probs = NULL;
    if (adjusted_probs == NULL)
        adjusted_probs = malloc_adjusted_probs();

    init_adjusted_probs(adjusted_probs, cur_trans_probs, cur_trans_prob_count);

    /* find any interactions */
    for (int i = 0; i < cur_states_count; ++i)
    {
        state_t *state = &states[cur_state_ids[i]];
        for (int iid = 0; iid < INTERACTION_COUNT; ++iid)
        {
            interaction_t *interaction = &interactions[iid];
            /* this condition doesn't look right */
            if (interaction->from_state_id == cur_state->id &&
                interaction->cur_state_id == state->id)
            {
                cur_interactions[cur_interaction_count] = interaction;
                cur_interaction_count += 1;
            }
        }
    }

    /* for every interaction */
    for (int i = 0; i < cur_interaction_count; ++i)
    {
        interaction_t *interaction = cur_interactions[i];
        float sum = 0.0;
        /* for every transition probablility */
        for (int i = 0; i < cur_trans_prob_count; ++i)
        {
            trans_prob_t *trans_prob = &adjusted_probs[i];
            if (trans_prob->from_id == interaction->from_state_id &&
                trans_prob->to_id == interaction->to_state_id)
            {
                trans_prob->prob *= interaction->adjustment;
                sum += trans_prob->prob;
            }
        }
        float new_adj_factor = 1.0 / sum;
        /* for every transition probablility */
        for (int i = 0; i < cur_trans_prob_count; ++i)
        {
            adjusted_probs[i].prob *= new_adj_factor;
        }
    }

    state_t *new_state = pickState(adjusted_probs, cur_trans_prob_count);
    person_add_state(person_id, cycle_id, new_state);
}

/* used for random access to models */
uint model_indices[MODEL_COUNT];

void run_model(void)
{
    /* init model_indices */
    for (int i = 0; i < MODEL_COUNT; ++i)
        model_indices[i] = i;

    /* for every cycle, starting at 1 (first real cycle) */
    for (int cid = 1; cid < CYCLE_COUNT; ++cid)
    {
        DEBUG_PRINT("run_model(): cycle %d start\n", cid);
        /* for every person */
        for (int pid = 0; pid < PERSON_COUNT; ++pid)
        {
            /* for every model, in random order */
            // suffle(model_indices, MODEL_COUNT);
            for (int i = 0; i < MODEL_COUNT; ++i)
            {
                id_t mid = model_indices[i];
                run_cycle(cid, pid, mid);
            }
        }
    }
}

void dumpCSVs(void)
{
    mkdir("tmp", ACCESSPERMS);
    FILE *f = fopen("tmp/masters.csv", "w");

    if (f == NULL) {
        printf("dumpCSVs(): fopen(): %s\n", strerror(errno));
        exit(1);
    }

    fprintf(f, "cycle_id,person_id,state_id,model_id\n");

    for (int i = 0; i < master_count; ++i)
    {
        fprintf(f, "%d,%d,%d,%d\n",
            masters[i].cycle_id,
            masters[i].person_id,
            masters[i].state_id,
            masters[i].model_id
        );
    }
    fclose(f);
}

int main(int argc, char const *argv[])
{
    clock_t start, end;

    srand(time(NULL));

    printf("init ...\n");
    start = clock();

    init_indexes();
    init_models();
    init_masters();
    init_world();

    printf("modelling ...\n");

    run_model();

    end = clock();

    printf("done\n");
    printf("elapsed time: %.9fs\n", (end - start)/1000000.0);
    printf("writing results ...\n");

    dumpCSVs();

    return 0;
}
