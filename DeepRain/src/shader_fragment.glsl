#version 330 core

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpola��o da posi��o global e a normal de cada v�rtice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;

// Posi��o do v�rtice atual no sistema de coordenadas local do modelo.
in vec4 position_model;

// Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
in vec2 texcoords;

// Matrizes computadas no c�digo C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador que define qual objeto est� sendo desenhado no momento
#define SPHERE 0
#define BUNNY  1
#define PLANE  2
#define LIBERTY  3
#define MONSTER 4
uniform int object_id;

// Par�metros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

// Vari�veis para acesso das imagens de textura
uniform sampler2D TextureImage0;
uniform sampler2D TextureImage1;
uniform sampler2D TextureImage2;
uniform sampler2D TextureImage3;

// O valor de sa�da ("out") de um Fragment Shader � a cor final do fragmento.
out vec4 color;

// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

void main()
{
    // Obtemos a posi��o da c�mera utilizando a inversa da matriz que define o
    // sistema de coordenadas da c�mera.
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    // O fragmento atual � coberto por um ponto que percente � superf�cie de um
    // dos objetos virtuais da cena. Este ponto, p, possui uma posi��o no
    // sistema de coordenadas global (World coordinates). Esta posi��o � obtida
    // atrav�s da interpola��o, feita pelo rasterizador, da posi��o de cada
    // v�rtice.
    vec4 p = position_world;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada v�rtice.
    vec4 n = normalize(normal);

    // Vetor que define o sentido da fonte de luz em rela��o ao ponto atual.
    vec4 l = normalize(vec4(1.0, 1.0, 0.5, 0.0));
    // Ponto onde est� localizada a fonte de luz spotlight
    vec4 l_spot = vec4(0.0f, 2.0f, 1.0f, 1.0f);
    // Vetor que define o sentido da fonte de luz spotlight em rela��o ao ponto atual
    vec4 L = normalize(l_spot - p);
    // Vetor que define o sentido da c�mera em rela��o ao ponto atual.
    vec4 v = normalize(camera_position - p);
    // Vetor que define o sentido da ilumina��o spotlight
    vec4 V = normalize(vec4(0.0f, -1.0f, 0.0f, 0.0f));

    // Vetor que define o sentido da reflex�o especular ideal.
    // vec4 r = vec4(0.0,0.0,0.0,0.0);
    // PARA SPOTLIGHT
    // vec4 r = -L+2*n*(dot(n, L));
    // PARA ILUMINA��O LOCAL
    vec4 r = -l+2*n*(dot(n, l));

    // Coordenadas de textura U e V
    float p_U = 0.0;
    float p_V = 0.0;

    // Par�metros que definem as propriedades espectrais da superf�cie
    vec3 Kd; // Reflet�ncia difusa
    vec3 Ks; // Reflet�ncia especular
    vec3 Ka; // Reflet�ncia ambiente
    float q; // Expoente especular para o modelo de ilumina��o de Phong

    if ( object_id == SPHERE )
    {
        // Propriedades espectrais da esfera
        // Kd = vec3(0.8, 0.4, 0.08);
        // Ks = vec3(0.0, 0.0, 0.0);
        // Ka = vec3(0.4, 0.2, 0.04);
        // q = 1.0;

        // PARA TEXTURA
        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;
        vec4 p_line = bbox_center + (position_model - bbox_center)/(length(position_model - bbox_center));
        vec4 vecp = p_line - bbox_center;

        p_U = (atan(vecp[0], vecp[2]) + M_PI)/(2*M_PI);
        p_V = (asin(vecp[1]) + M_PI_2)/M_PI;
    }
    else if ( object_id == BUNNY )
    {
        // Propriedades espectrais do coelho
        // Kd = vec3(0.008, 0.4, 0.8);
        // Ks = vec3(0.8, 0.8, 0.8);
        // Ka = vec3(0.004 ,0.2, 0.4);
        // q = 32.0;

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
    else if ( object_id == PLANE )
    {
        // Propriedades espectrais do plano
        // Kd = vec3(0.2, 0.2, 0.2);
        // Ks = vec3(0.3, 0.3, 0.3);
        // Ka = vec3(0.0, 0.0, 0.0);
        // q = 20.0;

        // PARA TEXTURA
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        p_U = texcoords.x;
        p_V = texcoords.y;
    }
    else if ( object_id == LIBERTY )
    {
        // Propriedades espectrais da liberdade
        // Kd = vec3(0.008, 0.4, 0.8);
        // Ks = vec3(0.8, 0.8, 0.8);
        // Ka = vec3(0.004 ,0.2, 0.4);
        // q = 32.0;

        //liberty_p_U = texcoords_liberty.x;
        //liberty_p_V = texcoords_liberty.y;
    }
    else if ( object_id == MONSTER )
    {
        // Propriedades espectrais do coelho
        // Kd = vec3(0.8, 0.4, 0.08);
        // Ks = vec3(0.0, 0.0, 0.0);
        // Ka = vec3(0.4, 0.2, 0.04);
        // q = 64.0;

        // PARA TEXTURA
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        p_U = texcoords.x;
        p_V = texcoords.y;
    }
    else // Objeto desconhecido = preto
    {
        Kd = vec3(0.0, 0.0, 0.0);
        Ks = vec3(0.0, 0.0, 0.0);
        Ka = vec3(0.0, 0.0, 0.0);
        q = 1.0;
    }

    // Obtemos a reflet�ncia difusa a partir da leitura da imagem TextureImage0
    vec3 Kd0 = texture(TextureImage0, vec2(p_U, p_V)).rgb;
    vec3 Kd1 = texture(TextureImage1, vec2(p_U, p_V)).rgb;
    vec3 Kd2 = texture(TextureImage2, vec2(p_U, p_V)).rgb;
    //vec3 Kd3 = texture(TextureImage3, vec2(liberty_p_U, liberty_p_V)).rgb;

    // Espectro da fonte de ilumina��o
    vec3 I = vec3(1.0, 1.0, 1.0);

    // Espectro da luz ambiente
    vec3 Ia = vec3(0.2, 0.2, 0.2);

    // Termo ambiente
    vec3 ambient_term = Ka*Ia;

    // Termo difuso utilizando a lei dos cossenos de Lambert
    // PARA ILUMINA��O LOCAL
    // vec3 lambert_diffuse_term = Kd*I*max(0.0, dot(n,l));
    // PARA SPOTLIGHT
    // vec3 lambert_diffuse_term = Kd*I*max(0.0, dot(n, L));
    // PARA TEXTURA
    float lambert = max(0,dot(n,l));

    // Termo especular utilizando o modelo de ilumina��o de Phong
    // PARA ILUMINA��O LOCAL
    // vec3 phong_specular_term  = Ks*I*max(0.0, pow(dot(r, v), q));
    // PARA SPOTLIGHT
    // vec3 phong_specular_term  = Ks*I*max(0.0, pow(dot(r, v), q));

    // PARA SPOTLIGHT
    // float alpha = 3.141592/6;
    // float beta = dot((normalize(p - l_spot)), V);

    // NOTE: Se voc� quiser fazer o rendering de objetos transparentes, �
    // necess�rio:
    // 1) Habilitar a opera��o de "blending" de OpenGL logo antes de realizar o
    //    desenho dos objetos transparentes, com os comandos abaixo no c�digo C++:
    //      glEnable(GL_BLEND);
    //      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // 2) Realizar o desenho de todos objetos transparentes *ap�s* ter desenhado
    //    todos os objetos opacos; e
    // 3) Realizar o desenho de objetos transparentes ordenados de acordo com
    //    suas dist�ncias para a c�mera (desenhando primeiro objetos
    //    transparentes que est�o mais longe da c�mera).
    // Alpha default = 1 = 100% opaco = 0% transparente
    color.a = 1;
   // monster_color.a = 1;
    //liberty_color.a = 1;

    // Cor final do fragmento calculada com uma combina��o dos termos difuso,
    // especular, e ambiente. Veja slide 129 do documento Aula_17_e_18_Modelos_de_Iluminacao.pdf.
    // PARA ILUMINA��O LOCAL
    // color.rgb = lambert_diffuse_term + ambient_term + phong_specular_term;
    // PARA TEXTURA
    //color.rgb = Kd0 * (lambert + 0.01) + (Kd1 * 0.25);
    color.rgb = Kd2 * (lambert + 0.01);
    //liberty_color.rgb = Kd3 * (lambert + 0.01);

    // PARA SPOTLIGHT
    /*if (beta > cos(alpha))
    {
        color.rgb = lambert_diffuse_term + ambient_term + phong_specular_term;
    }
    else
    {
        color.rgb = ambient_term;
    }*/

    // Cor final com corre��o gamma, considerando monitor sRGB.
    // Veja https://en.wikipedia.org/w/index.php?title=Gamma_correction&oldid=751281772#Windows.2C_Mac.2C_sRGB_and_TV.2Fvideo_standard_gammas
    color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    //monster_color.rgb = pow(monster_color.rgb, vec3(1.0,1.0,1.0)/2.2);
    //liberty_color.rgb = pow(liberty_color.rgb, vec3(1.0,1.0,1.0)/2.2);

}

