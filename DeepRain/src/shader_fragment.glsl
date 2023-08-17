#version 330 core

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpolação da posição global e a normal de cada vértice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;

// Posição do vértice atual no sistema de coordenadas local do modelo.
in vec4 position_model;

// Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
in vec2 texcoords;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador que define qual objeto está sendo desenhado no momento
#define SKYBOX 0
#define BUNNY  1
#define PLANE  2
#define LIBERTY  3
#define MONSTER 4
#define ROCK 5
#define FLYMONSTER 6
#define SPACESHIP 7
#define MOUNT 8
#define BULLETS 9
#define HITBOX 10

uniform int object_id;

// Parâmetros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

// Variáveis para acesso das imagens de textura
uniform sampler2D TextureImage0; //MAP
uniform sampler2D TextureImage1; //LIGHTS MAP
uniform sampler2D TextureImage2; //MONSTER
uniform sampler2D TextureImage3; //STATUE
uniform sampler2D TextureImage4; //GRASS
uniform sampler2D TextureImage5; //SKY
uniform sampler2D TextureImage6; //ROCK
uniform sampler2D TextureImage7; //FLYMONSTER
uniform sampler2D TextureImage8; //SPACESHIP
uniform sampler2D TextureImage9; //MOUNT
uniform sampler2D TextureImage10; //BULLETS

// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec4 color;

// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

void main()
{
    // Obtemos a posição da câmera utilizando a inversa da matriz que define o
    // sistema de coordenadas da câmera.
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    // O fragmento atual é coberto por um ponto que percente à superfície de um
    // dos objetos virtuais da cena. Este ponto, p, possui uma posição no
    // sistema de coordenadas global (World coordinates). Esta posição é obtida
    // através da interpolação, feita pelo rasterizador, da posição de cada
    // vértice.
    vec4 p = position_world;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada vértice.
    vec4 n = normalize(normal);

    // Vetor que define o sentido da fonte de luz em relação ao ponto atual.
    vec4 l = normalize(vec4(0.0, 1.0, 0.5, 0.0));
    // Vetor que define o sentido da câmera em relação ao ponto atual.
    vec4 v = normalize(camera_position - p);
    // Half vector para cálculo do termo de Blinn-Phong
    vec4 h = normalize(v+l);


    // Vetor que define o sentido da reflexão especular ideal.
    vec4 r = -l+2*n*(dot(n, l));

    // Coordenadas de textura U e V
    float p_U = 0.0;
    float p_V = 0.0;

    // Parâmetros que definem as propriedades espectrais da superfície
    vec3 Kd; // Refletância difusa
    vec3 Ks; // Refletância especular
    vec3 Ka; // Refletância ambiente
    float q; // Expoente especular para o modelo de iluminação de Phong

    if ( object_id == SKYBOX || object_id == BULLETS || object_id == HITBOX)
    {
        // PARA TEXTURA
        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;
        vec4 p_line = bbox_center + (position_model - bbox_center)/(length(position_model - bbox_center));
        vec4 vecp = p_line - bbox_center;

        p_U = (atan(vecp[0], vecp[2]) + M_PI)/(2*M_PI);
        p_V = (asin(vecp[1]) + M_PI_2)/M_PI;
    }
    else if ( object_id == BUNNY)
    {
        Kd = vec3(0.008,0.4,0.8);
        Ks = vec3(0.8,0.8,0.8);
        Ka = vec3(0.004,0.2,0.4);
        q = 32.0;

        // PARA TEXTURA
        float minx = bbox_min.x;
        float maxx = bbox_max.x;

        float miny = bbox_min.y;
        float maxy = bbox_max.y;

        float minz = bbox_min.z;
        float maxz = bbox_max.z;

        p_U = (position_model.x - minx)/(maxx - minx);
        p_V = (position_model.y - miny)/(maxy - miny);

    }
    else if ( object_id == LIBERTY || object_id == MONSTER ||
              object_id == ROCK || object_id == FLYMONSTER ||
              object_id == SPACESHIP || object_id == MOUNT )
    {
        // Coordenadas de textura da estatua, monstro ou pedra, obtidas dos arquivos OBJ.
        p_U = texcoords.x;
        p_V = texcoords.y;
    }
    else if ( object_id == PLANE )
    {
        // Coordenadas do plano, obtidas dos arquivos OBJ.
        // multiplicação por 10 para forçar as coordenadas a sairem do intervalo [0, 1]
        // e serem repetidas pelo parâmetro de texture wrapping GL_MIRRORED_REPEAT
        p_U = texcoords.x * 10;
        p_V = texcoords.y * 10;
    }
    else // Objeto desconhecido = preto
    {
        p_U = 0.0f;
        p_V = 0.0f;
    }

    // Obtemos a refletância difusa a partir da leitura da imagem TextureImage0
    vec3 Kd0 = texture(TextureImage0, vec2(p_U, p_V)).rgb;
    vec3 Kd1 = texture(TextureImage1, vec2(p_U, p_V)).rgb;

    if (object_id == MONSTER)
    {
        Kd0 = texture(TextureImage2, vec2(p_U, p_V)).rgb;
    }
    else if (object_id == LIBERTY)
    {
        Kd0 = texture(TextureImage3, vec2(p_U, p_V)).rgb;
    }
    else if (object_id == PLANE)
    {
        Kd0 = texture(TextureImage4, vec2(p_U, p_V)).rgb;
    }
    else if (object_id == SKYBOX || object_id == HITBOX)
    {
        Kd0 = texture(TextureImage5, vec2(p_U, p_V)).rgb;
    }
    else if (object_id == ROCK)
    {
        Kd0 = texture(TextureImage6, vec2(p_U, p_V)).rgb;
    }
    else if (object_id == FLYMONSTER)
    {
        Kd0 = texture(TextureImage7, vec2(p_U, p_V)).rgb;
    }
    else if (object_id == SPACESHIP)
    {
        Kd0 = texture(TextureImage8, vec2(p_U, p_V)).rgb;
    }
    else if (object_id == MOUNT)
    {
        Kd0 = texture(TextureImage9, vec2(p_U, p_V)).rgb;
    }
    else if (object_id == BULLETS)
    {
        Kd0 = texture(TextureImage10, vec2(p_U, p_V)).rgb;
    }

    // Espectro da fonte de iluminação
    vec3 I = vec3(1.0, 1.0, 1.0);

    // Espectro da luz ambiente
    vec3 Ia = vec3(0.2, 0.2, 0.2);

    // Termo ambiente
    vec3 ambient_term = Ka*Ia;

    // Termo difuso utilizando a lei dos cossenos de Lambert
    // PARA ILUMINAÇÃO LOCAL
    // vec3 lambert_diffuse_term = Kd*I*max(0.0, dot(n,l));
    float lambert = max(0, dot(n, l));
    float invlambert = max(0, dot(-n, l));

    // Termo especular utilizando o modelo de iluminação de Phong
    // PARA ILUMINAÇÃO LOCAL
    // vec3 phong_specular_term  = Ks*I*max(0.0, pow(dot(r, v), q));
    vec3 blinn_phong_specular_term = Ks*I*max(0.0, pow(dot(n, h), q));

    // NOTE: Se você quiser fazer o rendering de objetos transparentes, é
    // necessário:
    // 1) Habilitar a operação de "blending" de OpenGL logo antes de realizar o
    //    desenho dos objetos transparentes, com os comandos abaixo no código C++:
    //      glEnable(GL_BLEND);
    //      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // 2) Realizar o desenho de todos objetos transparentes *após* ter desenhado
    //    todos os objetos opacos; e
    // 3) Realizar o desenho de objetos transparentes ordenados de acordo com
    //    suas distâncias para a câmera (desenhando primeiro objetos
    //    transparentes que estão mais longe da câmera).
    // Alpha default = 1 = 100% opaco = 0% transparente
    if (object_id == HITBOX)
        color.a = 0;
    else
        color.a = 1;

    // Cor final do fragmento calculada com uma combinação dos termos difuso,
    // especular, e ambiente. Veja slide 129 do documento Aula_17_e_18_Modelos_de_Iluminacao.pdf.
    // PARA ILUMINAÇÃO LOCAL
    // color.rgb = lambert_diffuse_term + ambient_term + phong_specular_term;
    // PARA TEXTURA
    if (object_id == BUNNY)
        color.rgb = Kd0 * (lambert + 0.1) + Kd1 * (invlambert + 0.25) + blinn_phong_specular_term;
    else
        color.rgb = Kd0 * (lambert + 0.1);

    // Cor final com correção gamma, considerando monitor sRGB.
    // Veja https://en.wikipedia.org/w/index.php?title=Gamma_correction&oldid=751281772#Windows.2C_Mac.2C_sRGB_and_TV.2Fvideo_standard_gammas
    color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);

}

