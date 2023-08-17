// Headers da biblioteca GLM: cria��o de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include "iostream"
using namespace std;

//FONTE: https://chat.openai.com/share/8014b574-dd5a-46d0-b938-9bfe7b7a19ba
bool ColisaoPontoEsfera(glm::vec4 ponto, glm::vec4 centro_esfera, float raio)
{
    float distancia = sqrt(pow(ponto.x - centro_esfera.x, 2)
                         + pow(ponto.y - centro_esfera.y, 2)
                         + pow(ponto.z - centro_esfera.z, 2));

    // Verifica se a dist�ncia � menor ou igual ao raio da esfera
    return distancia <= raio;
}

//FONTE: https://chat.openai.com/share/8014b574-dd5a-46d0-b938-9bfe7b7a19ba
bool ColisaoPontoPlano(glm::vec4 ponto, glm::vec4 normal_plano) {

    float distancia = (normal_plano.x * ponto.x + normal_plano.y * ponto.y + normal_plano.z * ponto.z) /
                       sqrt(normal_plano.x * normal_plano.x + normal_plano.y * normal_plano.y + normal_plano.z * normal_plano.z);

    // Verifica se o ponto est� do mesmo lado do plano em rela��o � normal do plano
    return distancia >= 0.0f;
}

//Pensada pelos membros da dupla com base nas duas fun��es acima
bool ColisaoEsferaEsfera(glm::vec4 centro_esfera_1, float raio_1, glm::vec4 centro_esfera_2, float raio_2)
{
    float distancia = sqrt(pow(centro_esfera_1.x - centro_esfera_2.x, 2)
                         + pow(centro_esfera_1.y - centro_esfera_2.y, 2)
                         + pow(centro_esfera_1.z - centro_esfera_2.z, 2));

    return distancia <= (raio_1 + raio_2);
}



