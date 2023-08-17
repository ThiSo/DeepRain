// Headers da biblioteca GLM: cria��o de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include "iostream"
using namespace std;

// Fun��o que calc�la uma curva de bezi�r de grau 3
// Recebe como par�metros o fator de interpola��o e os pontos de controle
glm::vec3 CalculaBezier(float interpol, glm::vec3 pc_1, glm::vec3 pc_2, glm::vec3 pc_3, glm::vec3 pc_4)
{
    float bern = 1.0f - interpol;

    // Polinomios de Bernstein
    glm::vec3 c_t = pow(bern, 3.0f) * pc_1;
    c_t += 3.0f * pow(bern, 2.0f) * interpol * pc_2;
    c_t += 3.0f * bern * pow(interpol, 2.0f) * pc_3;
    c_t += pow(interpol, 3.0f) * pc_4;

    return c_t;
}
