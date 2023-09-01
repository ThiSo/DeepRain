// Headers das funções de cálculo de colisão
bool ColisaoPontoEsfera(glm::vec4 ponto, glm::vec4 centro_esfera, float raio);
bool ColisaoPontoPlano(glm::vec4 ponto, glm::vec4 normal_plano);
bool ColisaoEsferaEsfera(glm::vec4 centro_esfera_1, float raio_1, glm::vec4 centro_esfera_2, float raio_2);
bool ColisaoPontoCubo(glm::vec4 ponto, glm::vec3 cubo_min, glm::vec3 cubo_max);
