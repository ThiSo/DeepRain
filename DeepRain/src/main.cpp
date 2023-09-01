//     Universidade Federal do Rio Grande do Sul
//             Instituto de Inform�tica
//       Departamento de Inform�tica Aplicada
//
//    INF01047 Fundamentos de Computa��o Gr�fica
//               Prof. Eduardo Gastal
//
//                   DeepRain v1
//

// Arquivos "headers" padr�es de C podem ser inclu�dos em um
// programa C++, sendo necess�rio somente adicionar o caractere
// "c" antes de seu nome, e remover o sufixo ".h". Exemplo:
//    #include <stdio.h> // Em C
//  vira
//    #include <cstdio> // Em C++
//
#define STB_IMAGE_IMPLEMENTATION

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>

// Headers abaixo s�o espec�ficos de C++
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Cria��o de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Cria��o de janelas do sistema operacional

// Headers da biblioteca GLM: cria��o de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

// Headers da biblioteca para carregar modelos obj
#include <stb_image.h>
#include <tiny_obj_loader.h>

// Headers locais, definidos na pasta "include/"
#include "utils.h"
#include "matrices.h"

// Defines para os objetos
#define SKYBOX 0
#define BUNNY  1
#define PLANE  2
#define LIBERTY 3
#define MONSTER 4
#define ROCK 5
#define FLYMONSTER 6
#define SPACESHIP 7
#define MOUNT 8
#define BULLETS 9
#define HITBOX 10
#define PIECE 11
#define TREE 12
#define BOSS 13
#define BATTERY 14
#define AMMO 15
#define HEART 16
#define GUN 17
#define CAPSULE 18

// Prints para debugging
#include "iostream"
using namespace std;

#include "collisions.h"
#include "bezier.h"

#define M_PI   3.14159265358979323846

// Estrutura que representa um modelo geom�trico carregado a partir de um
// arquivo ".obj". Veja https://en.wikipedia.org/wiki/Wavefront_.obj_file .
struct ObjModel
{
    tinyobj::attrib_t                 attrib;
    std::vector<tinyobj::shape_t>     shapes;
    std::vector<tinyobj::material_t>  materials;

    // Este construtor l� o modelo de um arquivo utilizando a biblioteca tinyobjloader.
    // Veja: https://github.com/syoyo/tinyobjloader
    ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true)
    {
        printf("Carregando objetos do arquivo \"%s\"...\n", filename);

        // Se basepath == NULL, ent�o setamos basepath como o dirname do
        // filename, para que os arquivos MTL sejam corretamente carregados caso
        // estejam no mesmo diret�rio dos arquivos OBJ.
        std::string fullpath(filename);
        std::string dirname;
        if (basepath == NULL)
        {
            auto i = fullpath.find_last_of("/");
            if (i != std::string::npos)
            {
                dirname = fullpath.substr(0, i+1);
                basepath = dirname.c_str();
            }
        }

        std::string warn;
        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, basepath, triangulate);

        if (!err.empty())
            fprintf(stderr, "\n%s\n", err.c_str());

        if (!ret)
            throw std::runtime_error("Erro ao carregar modelo.");

        for (size_t shape = 0; shape < shapes.size(); ++shape)
        {
            if (shapes[shape].name.empty())
            {
                fprintf(stderr,
                        "*********************************************\n"
                        "Erro: Objeto sem nome dentro do arquivo '%s'.\n"
                        "Veja https://www.inf.ufrgs.br/~eslgastal/fcg-faq-etc.html#Modelos-3D-no-formato-OBJ .\n"
                        "*********************************************\n",
                    filename);
                throw std::runtime_error("Objeto sem nome.");
            }
            printf("- Objeto '%s'\n", shapes[shape].name.c_str());
        }

        printf("OK.\n");
    }
};


// Declara��o de fun��es utilizadas para pilha de matrizes de modelagem.
void PushMatrix(glm::mat4 M);
void PopMatrix(glm::mat4& M);

// Declara��o de v�rias fun��es utilizadas em main().  Essas est�o definidas
// logo ap�s a defini��o de main() neste arquivo.
void BuildTrianglesAndAddToVirtualScene(ObjModel*); // Constr�i representa��o de um ObjModel como malha de tri�ngulos para renderiza��o
GLuint BuildTrianglesForCrosshair(); // Constr�i tri�ngulos para renderiza��o
GLuint BuildTrianglesForGameOverScreen(); // Constr�i tri�ngulos para renderiza��o
void ComputeNormals(ObjModel* model); // Computa normais de um ObjModel, caso n�o existam.
void LoadShadersFromFiles(); // Carrega os shaders de v�rtice e fragmento, criando um programa de GPU
void LoadTextureImage(const char* filename); // Fun��o que carrega imagens de textura
void DrawVirtualObject(const char* object_name); // Desenha um objeto armazenado em g_VirtualScene
GLuint LoadShader_Vertex(const char* filename);   // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename); // Carrega um fragment shader
void LoadShader(const char* filename, GLuint shader_id); // Fun��o utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU

// Declara��o de fun��es auxiliares para renderizar texto dentro da janela
// OpenGL. Estas fun��es est�o definidas no arquivo "textrendering.cpp".
void TextRendering_Init();
float TextRendering_LineHeight(GLFWwindow* window);
float TextRendering_CharWidth(GLFWwindow* window);
void TextRendering_PrintString(GLFWwindow* window, const std::string &str, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrix(GLFWwindow* window, glm::mat4 M, float x, float y, float scale = 1.0f);
void TextRendering_PrintVector(GLFWwindow* window, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProduct(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductMoreDigits(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductDivW(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);

// Fun��es abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informa��es do programa. Definidas ap�s main().
void TextRendering_ShowModelViewProjection(GLFWwindow* window, glm::mat4 projection, glm::mat4 view, glm::mat4 model, glm::vec4 p_model);
void TextRendering_ShowBullets(GLFWwindow* window);
void TextRendering_ShowLifes(GLFWwindow* window);
void TextRendering_ShowPieces(GLFWwindow* window);
void TextRendering_ShowGameOver(GLFWwindow* window);
void TextRendering_ShowWin(GLFWwindow* window);
void TextRendering_ShowTime(GLFWwindow* window);
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);
void TextRendering_ShowPoints(GLFWwindow* window, int points);
void TextRendering_ShowBuyUpgrade(GLFWwindow* window, int points);
void TextRendering_ShowMessageExtraLife(GLFWwindow* window);
void TextRendering_ShowMessageIncDamage(GLFWwindow* window);
void TextRendering_ShowMessageIncSpeed(GLFWwindow* window);
void TextRendering_ShowMessageInsufficientPoints(GLFWwindow* window);

// Fun��es callback para comunica��o com o sistema operacional e intera��o do
// usu�rio. Veja mais coment�rios nas defini��es das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// Definimos uma estrutura que armazenar� dados necess�rios para renderizar
// cada objeto da cena virtual.
struct SceneObject
{
    std::string  name;        // Nome do objeto
    size_t       first_index; // �ndice do primeiro v�rtice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    size_t       num_indices; // N�mero de �ndices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum       rendering_mode; // Modo de rasteriza��o (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint       vertex_array_object_id; // ID do VAO onde est�o armazenados os atributos do modelo
    glm::vec3    bbox_min; // Axis-Aligned Bounding Box do objeto
    glm::vec3    bbox_max;
};

// Abaixo definimos vari�veis globais utilizadas em v�rias fun��es do c�digo.

// A cena virtual � uma lista de objetos nomeados, guardados em um dicion�rio
// (map).  Veja dentro da fun��o BuildTrianglesAndAddToVirtualScene() como que s�o inclu�dos
// objetos dentro da vari�vel g_VirtualScene, e veja na fun��o main() como
// estes s�o acessados.
std::map<std::string, SceneObject> g_VirtualScene;

// Pilha que guardar� as matrizes de modelagem.
std::stack<glm::mat4>  g_MatrixStack;

// Classe / Vector para o jogador ////////////

class Player {
    public:
        glm::vec4 position;
        float speed = 10.0f;
        bool is_alive = true;
        int damage = 1;
        int lifes = 3;
        int points = 0;
};

std::vector<Player> player;

int num_lifes = 3;

// Classe / Vector para armazenar proj�teis //

class Projectile {
    public:
        glm::vec4 position;
        glm::vec4 speed;
        bool is_active;
        float radius = 0.25f;

        Projectile() : is_active(false) {}
};

std::vector<Projectile> shot;

int num_shots = 6;
bool reload = false;

// Classe / Vector para os monstros //////////

class Monster {
    public:
        glm::vec4 position;
        glm::vec4 hitbox;
        bool is_alive = true;
        bool proximo = false;
        float speed = 2.0f;
        float angle = 0.0f;
        float radius = 1.5f;
        int lifes = 3;
};

std::vector<Monster> monster;

// Classe / Vector para o boss ///////////////

class Boss {
    public:
        glm::vec4 position;
        glm::vec4 hitbox;
        bool is_alive = false;
        float speed = 3.0f;
        float angle = 0.0f;
        float radius = 11.0f;
        int lifes = 3;
};

// Classe / Vector para a nave ///////////////

class Spaceship {
    public:
        glm::vec4 position;
        float radius = 1.5f;
        int lifes = 1;
};

// Classe / Vector para as pe�as  ////////////

class Piece {
    public:
        glm::vec4 position;
        glm::vec4 hitbox;
        float angle = 2 * M_PI;
        float radius = 0.8f;
        bool collected = false;
};

std::vector<Piece> piece;

int num_pieces = 0;

// Classe / Vector para as capsulas de upgrade  ////////////

class Capsule {
    public:
        glm::vec4 position;
        glm::vec4 hitbox;
        float angle = 2 * M_PI;
        float radius = 0.3f;
        int price = 100;
        bool colide = false;
};

std::vector<Capsule> capsule;




// def do vetor de indices
typedef GLubyte index_type;

// Raz�o de propor��o da janela (largura/altura). Veja fun��o FramebufferSizeCallback().
float g_ScreenRatio = 1.0f;

// �ngulos de Euler que controlam a rota��o de um dos cubos da cena virtual
float g_AngleX = 0.0f;
float g_AngleY = 0.0f;
float g_AngleZ = 0.0f;

// Booleanos para o pulo do jogador
bool jump = false;
bool go_down = false;

// "g_LeftMouseButtonPressed = true" se o usu�rio est� com o bot�o esquerdo do mouse
// pressionado no momento atual. Veja fun��o MouseButtonCallback().
bool g_LeftMouseButtonPressed = false;
bool g_RightMouseButtonPressed = false; // An�logo para bot�o direito do mouse
bool g_MiddleMouseButtonPressed = false; // An�logo para bot�o do meio do mouse

// Vari�veis que definem a c�mera em coordenadas esf�ricas, controladas pelo
// usu�rio atrav�s do mouse (veja fun��o CursorPosCallback()). A posi��o
// efetiva da c�mera � calculada dentro da fun��o main(), dentro do loop de
// renderiza��o.
float g_CameraTheta = 0.0f; // �ngulo no plano ZX em rela��o ao eixo Z
float g_CameraPhi = 0.0f;   // �ngulo em rela��o ao eixo Y
float g_CameraDistance = 3.5f; // Dist�ncia da c�mera para a origem

// Vari�vel que controla o tipo de proje��o utilizada: perspectiva ou ortogr�fica.
bool g_UsePerspectiveProjection = true;

// Vari�vel que controla se o texto informativo ser� mostrado na tela.
bool g_ShowInfoText = true;

// Vari�veis que definem um programa de GPU (shaders). Veja fun��o LoadShadersFromFiles().
GLuint g_GpuProgramID = 0;
GLint g_model_uniform;
GLint g_view_uniform;
GLint g_projection_uniform;
GLint g_object_id_uniform;
GLint g_bbox_min_uniform;
GLint g_bbox_max_uniform;

// Booleanos para movimenta��o e camera lock
bool tecla_W_pressionada = false;
bool tecla_A_pressionada = false;
bool tecla_S_pressionada = false;
bool tecla_D_pressionada = false;
bool tecla_E_pressionada = false;

// Vari�veis para atualiza��o de posi��o e c�mera
float delta_t = 0.0f;
float g_Theta = 3.141592f / 4;
float g_Phi = 3.141592f / 6;

// Vetores da c�mera
glm::vec4 camera_view_vector;
glm::vec4 camera_up_vector;
glm::vec4 prev_view;
glm::vec4 prev_pos;
glm::vec4 direction;

// Pontos de posi��o do jogador
glm::vec4 camera_position_c = glm::vec4(0.0f, 1.0f, 2.0f, 1.0f);

// Vari�veis para o monstro com movimenta��o dada por curva de bezier
bool ciclo_voo = true;

// Vari�vel para a c�mera lookat que foca no boss spawnando
bool lookat_boss = false;

// N�mero de texturas carregadas pela fun��o LoadTextureImage()
GLuint g_NumLoadedTextures = 0;

int main(int argc, char* argv[])
{
    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com OpenGL.
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos o callback para impress�o de erros da GLFW no terminal
    glfwSetErrorCallback(ErrorCallback);

    // Pedimos para utilizar OpenGL vers�o 3.3 (ou superior)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Pedimos para utilizar o perfil "core", isto �, utilizaremos somente as
    // fun��es modernas de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criamos uma janela do sistema operacional, com 800 colunas e 600 linhas
    // de pixels, e com t�tulo "INF01047 ...".
    GLFWwindow* window;
    window = glfwCreateWindow(800, 600, "INF01047 - DeepRain", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos a fun��o de callback que ser� chamada sempre que o usu�rio
    // pressionar alguma tecla do teclado ...
    glfwSetKeyCallback(window, KeyCallback);
    // ... ou clicar os bot�es do mouse ...
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    // ... ou movimentar o cursor do mouse em cima da janela ...
    glfwSetCursorPosCallback(window, CursorPosCallback);
    // ... ou rolar a "rodinha" do mouse.
    glfwSetScrollCallback(window, ScrollCallback);

    // Indicamos que as chamadas OpenGL dever�o renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas fun��es definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Esconde o cursor e o inicia no centro da tela
    glfwSetCursorPos(window, 0.0f, 0.0f);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Definimos a fun��o de callback que ser� chamada sempre que a janela for
    // redimensionada, por consequ�ncia alterando o tamanho do "framebuffer"
    // (regi�o de mem�ria onde s�o armazenados os pixels da imagem).
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    FramebufferSizeCallback(window, 800, 600); // For�amos a chamada do callback acima, para definir g_ScreenRatio.
    glfwSetWindowPos(window, 250, 100);

    // Imprimimos no terminal informa��es sobre a GPU do sistema
    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    // Carregamos os shaders de v�rtices e de fragmentos que ser�o utilizados
    // para renderiza��o. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
    //
    LoadShadersFromFiles();

    // Carregamos duas imagens para serem utilizadas como textura
    LoadTextureImage("../../data/tc-earth_daymap_surface.jpg");      // TextureImage0
    LoadTextureImage("../../data/tc-earth_nightmap_citylights.gif"); // TextureImage1
    LoadTextureImage("../../data/tc-monster.jpg");                   // TextureImage2
    LoadTextureImage("../../data/tc-liberty.png");                   // TextureImage3
    LoadTextureImage("../../data/tc-grass.jpg");                     // TextureImage4
    LoadTextureImage("../../data/tc-skydome.jpg");                   // TextureImage5
    LoadTextureImage("../../data/tc-rock.jpg");                      // TextureImage6
    LoadTextureImage("../../data/tc-flymonster.jpg");                // TextureImage7
    LoadTextureImage("../../data/tc-spaceship.jpg");                 // TextureImage8
    LoadTextureImage("../../data/tc-mount.jpg");                     // TextureImage9
    LoadTextureImage("../../data/tc-bullet.jpg");                    // TextureImage10
    LoadTextureImage("../../data/tc-piece.png");                     // TextureImage11
    LoadTextureImage("../../data/tc-tree.jpg");                      // TextureImage12
    LoadTextureImage("../../data/tc-boss_metal.jpg");                // TextureImage13
    LoadTextureImage("../../data/tc-boss_body.jpg");                 // TextureImage14
    LoadTextureImage("../../data/tc-battery.jpg");                   // TextureImage15
    LoadTextureImage("../../data/tc-heart.jpg");                     // TextureImage16
    LoadTextureImage("../../data/tc-gun.jpg");                       // TextureImage17
    LoadTextureImage("../../data/tc-capsule.png");                       // TextureImage18



    // Constru�mos a representa��o de objetos geom�tricos atrav�s de malhas de tri�ngulos
    ObjModel spheremodel("../../data/sphere.obj");
    ComputeNormals(&spheremodel);
    BuildTrianglesAndAddToVirtualScene(&spheremodel);

    ObjModel bunnymodel("../../data/bunny.obj");
    ComputeNormals(&bunnymodel);
    BuildTrianglesAndAddToVirtualScene(&bunnymodel);

    ObjModel planemodel("../../data/plane.obj");
    ComputeNormals(&planemodel);
    BuildTrianglesAndAddToVirtualScene(&planemodel);

    ObjModel libertymodel("../../data/liberty.obj");
    ComputeNormals(&libertymodel);
    BuildTrianglesAndAddToVirtualScene(&libertymodel);

    ObjModel monstermodel("../../data/monster.obj");
    ComputeNormals(&monstermodel);
    BuildTrianglesAndAddToVirtualScene(&monstermodel);

    ObjModel rockmodel("../../data/rock.obj");
    ComputeNormals(&rockmodel);
    BuildTrianglesAndAddToVirtualScene(&rockmodel);

    ObjModel flymonstermodel("../../data/flymonster.obj");
    ComputeNormals(&flymonstermodel);
    BuildTrianglesAndAddToVirtualScene(&flymonstermodel);

    ObjModel shipmodel("../../data/spaceship.obj");
    ComputeNormals(&shipmodel);
    BuildTrianglesAndAddToVirtualScene(&shipmodel);

    ObjModel mountmodel("../../data/mount.obj");
    ComputeNormals(&mountmodel);
    BuildTrianglesAndAddToVirtualScene(&mountmodel);

    ObjModel bulletmodel("../../data/sphere.obj");
    ComputeNormals(&bulletmodel);
    BuildTrianglesAndAddToVirtualScene(&bulletmodel);

    ObjModel piecemodel("../../data/piece.obj");
    ComputeNormals(&piecemodel);
    BuildTrianglesAndAddToVirtualScene(&piecemodel);

    ObjModel treemodel("../../data/tree.obj");
    ComputeNormals(&treemodel);
    BuildTrianglesAndAddToVirtualScene(&treemodel);

    ObjModel bossmodel("../../data/boss.obj");
    ComputeNormals(&bossmodel);
    BuildTrianglesAndAddToVirtualScene(&bossmodel);

    ObjModel batterymodel("../../data/battery.obj");
    ComputeNormals(&batterymodel);
    BuildTrianglesAndAddToVirtualScene(&batterymodel);

    ObjModel ammomodel("../../data/ammo.obj");
    ComputeNormals(&ammomodel);
    BuildTrianglesAndAddToVirtualScene(&ammomodel);

    ObjModel heartmodel("../../data/heart.obj");
    ComputeNormals(&heartmodel);
    BuildTrianglesAndAddToVirtualScene(&heartmodel);

    ObjModel gunmodel("../../data/gun.obj");
    ComputeNormals(&gunmodel);
    BuildTrianglesAndAddToVirtualScene(&gunmodel);

    ObjModel capsulemodel("../../data/capsule.obj");
    ComputeNormals(&capsulemodel);
    BuildTrianglesAndAddToVirtualScene(&capsulemodel);

    if ( argc > 1 )
    {
        ObjModel model(argv[1]);
        BuildTrianglesAndAddToVirtualScene(&model);
    }

    // Constru�mos a representa��o de triangulos
    GLuint vertex_array_object_id_crosshair = BuildTrianglesForCrosshair();
    // GLuint vertex_array_object_id_gameoverscreen = BuildTrianglesForGameOverScreen();

    // Inicializamos o c�digo para renderiza��o de texto.
    TextRendering_Init();

    // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 23-34 do documento Aula_13_Clipping_and_Culling.pdf e slides 112-123 do documento Aula_14_Laboratorio_3_Revisao.pdf.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // adi��es pra atualiza��o da c�mera
    float prev_time = (float)glfwGetTime();

    // float monster_speed = 2.0f;
    float fly_monster_angle = 0.0f;

    float prevx_camera_position_c;
    float prevy_camera_position_c;
    float prevz_camera_position_c;

    int frame_counter_boss = 600; // Utilizado para definir por quantos frames a cutscene do boss deve durar
    float upgrade_massage_time; // Utilizado para definir por quantos frames a mensagem de qual upgrade foi adquirido deve ficar na tela
    int price = 100;

    bool init_counter_boss = false;
    bool gameOver = false;
    bool win = false;
    bool tp_boss = true;
    bool tp_end = true;
    bool show_message_1 = false; // Extra Life
    bool show_message_2 = false; // Inc Damage
    bool show_message_3 = false; // Inc Speed
    bool show_message_4 = false; // Insufficient Points
    bool canBuy = true; // Utilizado para definir se uma capsula de upgrades j� est� disponivel para uma nova compra

    float r, x, y, z;

    float t = 0.0f;     // Par�metro de interpola��o da curva de bezi�r

    // Inicializa��o do Player ////////////////////////////////////////////

    Player player;

    player.position = glm::vec4(camera_position_c.x, camera_position_c.y, camera_position_c.z, 1.0f);

    // Inicializa��o dos monstros /////////////////////////////////////////

    std::vector<glm::vec3> posVectorMonster;

    glm::vec3 monster_0_position = glm::vec3(-50.0f, 0.6f, 0.0f);
    glm::vec3 monster_1_position = glm::vec3(-30.0f, 0.6f, -30.0f);
    glm::vec3 monster_2_position = glm::vec3(-5.0f, 0.6f, -50.0f);
    glm::vec3 monster_3_position = glm::vec3(-10.0f, 0.6f, 30.0f);
    glm::vec3 monster_4_position = glm::vec3(-50.0f, 0.6f, 50.0f);
    glm::vec3 monster_5_position = glm::vec3(30.0f, 0.6f, -40.0f);
    glm::vec3 monster_6_position = glm::vec3(40.0f, 0.6f, -50.0f);
    glm::vec3 monster_7_position = glm::vec3(30.0f, 0.6f, 40.0f);
    glm::vec3 monster_8_position = glm::vec3(35.0f, 0.6f, 35.0f);
    glm::vec3 monster_9_position = glm::vec3(50.0f, 0.6f, 50.0f);

    posVectorMonster.push_back(monster_0_position);
    posVectorMonster.push_back(monster_1_position);
    posVectorMonster.push_back(monster_2_position);
    posVectorMonster.push_back(monster_3_position);
    posVectorMonster.push_back(monster_4_position);
    posVectorMonster.push_back(monster_5_position);
    posVectorMonster.push_back(monster_6_position);
    posVectorMonster.push_back(monster_7_position);
    posVectorMonster.push_back(monster_8_position);
    posVectorMonster.push_back(monster_9_position);

    for (const glm::vec3& monster_position : posVectorMonster) {

        Monster new_monster;

        new_monster.position = glm::vec4(monster_position.x, monster_position.y, monster_position.z, 1.0f);
        new_monster.hitbox = new_monster.position;
        monster.push_back(new_monster);
    }

    ///////////////////////////////////////////////////////////////////////

    // Inicializa��o das partes da nave ///////////////////////////////////

    std::vector<glm::vec3> posVectorPiece;

    srand(time(0));

    glm::vec3 piece_0_position = glm::vec3(-fmod(rand(),100.0f), 0.5f, fmod(rand(),100.0f));
    glm::vec3 piece_1_position = glm::vec3(fmod(rand(),100.0f), 0.5f, -fmod(rand(),100.0f));
    glm::vec3 piece_2_position = glm::vec3(-fmod(rand(),100.0f), 0.5f, fmod(rand(),100.0f));
    glm::vec3 piece_3_position = glm::vec3(fmod(rand(),100.0f), 0.5f, -fmod(rand(),100.0f));
    glm::vec3 piece_4_position = glm::vec3(fmod(rand(),100.0f), 0.5f, fmod(rand(),100.0f));

    posVectorPiece.push_back(piece_0_position);
    posVectorPiece.push_back(piece_1_position);
    posVectorPiece.push_back(piece_2_position);
    posVectorPiece.push_back(piece_3_position);
    posVectorPiece.push_back(piece_4_position);


    for(const glm::vec3& piece_position : posVectorPiece) {

        Piece new_piece;

        new_piece.position = glm::vec4(piece_position.x, piece_position.y, piece_position.z, 1.0f);
        new_piece.hitbox = new_piece.position;
        piece.push_back(new_piece);
    }

    ///////////////////////////////////////////////////////////////////////

    // Inicializa��o das capsulas de upgrade //////////////////////////////

    std::vector<glm::vec3> posVectorCapsule;

    glm::vec3 capsule_0_position = glm::vec3(-fmod(rand(),100.0f), -0.5f, fmod(rand(),100.0f));
    glm::vec3 capsule_1_position = glm::vec3(fmod(rand(),100.0f), -0.5f, -fmod(rand(),100.0f));
    glm::vec3 capsule_2_position = glm::vec3(-fmod(rand(),100.0f), -0.5f, fmod(rand(),100.0f));

    posVectorCapsule.push_back(capsule_0_position);
    posVectorCapsule.push_back(capsule_1_position);
    posVectorCapsule.push_back(capsule_2_position);


    for(const glm::vec3& capsule_position : posVectorCapsule) {

        Capsule new_capsule;

        new_capsule.position = glm::vec4(capsule_position.x, capsule_position.y, capsule_position.z, 1.0f);
        new_capsule.hitbox = new_capsule.position;
        capsule.push_back(new_capsule);
    }

    ///////////////////////////////////////////////////////////////////////

    // Inicializa��o do boss //////////////////////////////////////////////

    glm::vec3 boss_position = glm::vec3(100.0f, 11.0f, -100.0f);
    Boss boss;
    boss.position = glm::vec4(boss_position.x, boss_position.y, boss_position.z, 1.0f);
    boss.hitbox = boss.position;

    ///////////////////////////////////////////////////////////////////////

    // Inicializa��o da nave //////////////////////////////////////////////

    glm::vec3 spaceship_position = glm::vec3(0.0f, 0.0f, 7.5f);
    Spaceship spaceship;
    spaceship.position = glm::vec4(spaceship_position.x, spaceship_position.y, spaceship_position.z, 1.0f);

    ///////////////////////////////////////////////////////////////////////

    // Inicializa��o dos outros objetos ///////////////////////////////////

    glm::vec3 statue_position = glm::vec3(-60.0f, 10.0f, -60.0f);
    glm::vec3 mount_position = glm::vec3(55.0f, 18.0f, 30.0f);
    glm::vec3 tree_position = glm::vec3(-40.0f, 8.0f, 60.0f);
    glm::vec3 plane_position = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 fly_position;

    glm::vec4 bunny_position = glm::vec4(55.0f, 28.125f, 30.0f, 1.0f);
    bool bunny_alive = true;
    float bunny_radius = 2.0f;

    // Pontos de controle da primeira curva de bezier
    glm::vec3 ponto_controle_1 = glm::vec3(40.0f, 10.0f, 70.0f);
    glm::vec3 ponto_controle_2 = glm::vec3(52.0f, 35.0f, 40.0f);
    glm::vec3 ponto_controle_3 = glm::vec3(70.0f, 50.0f, 70.0f);
    glm::vec3 ponto_controle_4 = glm::vec3(85.0f, 20.0f, 40.0f);

    // Pontos de controle da segunda curva de bezier
    glm::vec3 ponto_controle_5 = glm::vec3(85.0f, 20.0f, 40.0f);
    glm::vec3 ponto_controle_6 = glm::vec3(60.0f, 35.0f, 10.0f);
    glm::vec3 ponto_controle_7 = glm::vec3(20.0f, 50.0f, 10.0f);
    glm::vec3 ponto_controle_8 = glm::vec3(40.0f, 10.0f, 70.0f);

    ///////////////////////////////////////////////////////////////////////

    // Inicializa��o das hitboxes /////////////////////////////////////////

    glm::vec4 hitbox_bunny = bunny_position;
    glm::vec4 hitbox_spaceship = glm::vec4(spaceship.position.x - 2.0f, spaceship.position.y, spaceship.position.z + 2.0f, 1.0f);

    ///////////////////////////////////////////////////////////////////////

    // Inicializa��o do vetor de movimento da camera para a cutscene

    glm::vec4 movementVec;

    // Ficamos em um loop infinito, renderizando, at� que o usu�rio feche a janela
    while (!glfwWindowShouldClose(window))
    {
        // Aqui executamos as opera��es de renderiza��o

        // Definimos a cor do "fundo" do framebuffer como branco.  Tal cor �
        // definida como coeficientes RGBA: Red, Green, Blue, Alpha; isto �:
        // Vermelho, Verde, Azul, Alpha (valor de transpar�ncia).
        // Conversaremos sobre sistemas de cores nas aulas de Modelos de Ilumina��o.
        //
        //           R     G     B     A
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        // "Pintamos" todos os pixels do framebuffer com a cor definida acima,
        // e tamb�m resetamos todos os pixels do Z-buffer (depth buffer).
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Pedimos para a GPU utilizar o programa de GPU criado acima (contendo
        // os shaders de v�rtice e fragmentos).
        glUseProgram(g_GpuProgramID);

        // Computamos a posi��o da c�mera utilizando coordenadas esf�ricas.  As
        // vari�veis g_CameraDistance, g_CameraPhi, e g_CameraTheta s�o
        // controladas pelo mouse do usu�rio. Veja as fun��es CursorPosCallback()
        // e ScrollCallback().
        r = g_CameraDistance;
        y = r*sin(g_CameraPhi);
        z = r*cos(g_CameraPhi)*cos(g_CameraTheta);
        x = r*cos(g_CameraPhi)*sin(g_CameraTheta);

        camera_up_vector   = glm::vec4(0.0f,1.0f,0.0f,0.0f); // Vetor "up" fixado para apontar para o "c�u" (eito Y global)

        if (win || gameOver)
        {
            camera_view_vector = glm::vec4(-500.0f, -100.0f, -500.0f, 1.0f) - player.position;
        }

        else if (frame_counter_boss == 0 && lookat_boss)
        {
            lookat_boss = false;
            camera_view_vector = prev_view;
            camera_position_c = prev_pos;
        }

        else if (lookat_boss == true && frame_counter_boss > 0)
        {
            prev_view = glm::vec4(x, y, -z, 0.0f);
            camera_view_vector = boss.position - player.position;
            x = camera_view_vector.x;
            y = camera_view_vector.y;
            z = camera_view_vector.z;
        }

        else
            camera_view_vector = glm::vec4(x, y, -z, 0.0f);            // Vetor "view", sentido para onde a c�mera est� virada

        glm::vec4 w = - camera_view_vector;
        glm::vec4 u = crossproduct(camera_up_vector, w);
        w = w / norm(w);
        u = u / norm(u);

        /////////////////// MOVIMENTA��O /////////////////////////////////////////

        if ((win || gameOver) && tp_end)
        {
            camera_position_c = glm::vec4(-500.0f, -95.0f, -492.0f, 1.0f);
            tp_end = false;
        }

        if (lookat_boss && tp_boss)
        {
            prev_pos = camera_position_c;
            camera_position_c = glm::vec4( 90.0f, 11.0f, -70.0f, 1.0f);
            movementVec = camera_position_c - player.position;
            tp_boss = false;
        }

        prevx_camera_position_c = camera_position_c.x;
        prevy_camera_position_c = camera_position_c.y;
        prevz_camera_position_c = camera_position_c.z;

        // Realiza movimenta��o do jogador
        if (!lookat_boss && !(win || gameOver))
        {

            if (tecla_W_pressionada)
                camera_position_c += -w * player.speed * delta_t;

            if (tecla_A_pressionada)
                camera_position_c += -u * player.speed * delta_t;

            if (tecla_S_pressionada)
                camera_position_c += w * player.speed * delta_t;

            if (tecla_D_pressionada)
                camera_position_c += u * player.speed * delta_t;
        }

        if (!tp_end)
        {
            // afasta a imagem lentamente do astronauta falecido
            camera_position_c -= camera_view_vector * (0.01f * player.speed) * delta_t;
        }

        if (lookat_boss && !tp_boss)
        {
            // afasta lentamente a c�mera do boss
            camera_position_c -= camera_view_vector * (0.01f * player.speed) * delta_t;
        }

        player.position = camera_position_c;

        //////////////////////////////////////////////////////////////////////////

        /////////////////// COLIS�ES /////////////////////////////////////////////

        // Checa colis�es ap�s o jogador ter se movimentado
        if (ColisaoPontoPlano(player.position, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)))
        {
            player.position.y = prevy_camera_position_c;
        }

        for (size_t i = 0; i < monster.size(); ++i) {

            if (ColisaoPontoEsfera(player.position, monster[i].position, monster[i].radius) && monster[i].lifes > 0)
            {
                // Se o jogador for atingido pelo monstro, perde uma vida
                player.lifes--;
                if (player.lifes <= 0){
                    player.is_alive = false;
                    gameOver = true;
                }
                num_lifes = player.lifes;

                // Reposiciona o jogador caso ele tome dano
                player.position.x = prevx_camera_position_c + 2.0f;
                player.position.z = prevz_camera_position_c + 2.0f;
            }

        }

        for (size_t i = 0; i < piece.size(); i++) {

            if (ColisaoPontoEsfera(player.position, piece[i].hitbox, piece[i].radius) && piece[i].collected == false) {
                num_pieces++;
                piece[i].collected = true;
            }
        }

        for (size_t i = 0; i < capsule.size(); ++i) {

            if (ColisaoPontoEsfera(player.position, capsule[i].hitbox, capsule[i].radius + 3.0f))
            {
                capsule[i].colide = true;
                price = capsule[i].price;

                if(tecla_E_pressionada && player.points >= capsule[i].price && canBuy)
                {
                    int randomNumber = fmod(rand(),3.0f);

                    // O player ganhou uma vida extra
                    if(randomNumber == 0.0f)
                    {
                        show_message_1 = true;
                        player.lifes++;
                        canBuy = false;
                        player.points -= capsule[i].price;
                    }

                    // O player ganhou um aumento de dano
                    else if(randomNumber == 1.0f)
                    {
                        show_message_2 = true;
                        player.damage++;
                        canBuy = false;
                        player.points -= capsule[i].price;
                    }

                    // O player ganhou um aumento na velocidade de movimento
                    else if(randomNumber == 2.0f)
                    {
                        show_message_3 = true;
                        player.speed++;
                        canBuy = false;
                        player.points -= capsule[i].price;
                    }
                    capsule[i].price += 100;
                    upgrade_massage_time = (float)glfwGetTime();
                }

                // O player n�o tem pontos o suficiente para adquirir um novo upgrade
                else if(tecla_E_pressionada && player.points < capsule[i].price && canBuy)
                {
                    show_message_4 = true;
                    canBuy = false;
                    upgrade_massage_time = (float)glfwGetTime();
                }
            }

            // Player deixou de colidir com a capsula
            else
            {
                capsule[i].colide = false;
            }
        }
        num_lifes = player.lifes;


        if (ColisaoPontoEsfera(player.position, hitbox_bunny, bunny_radius))
            bunny_alive = false;

        if (ColisaoPontoEsfera(player.position, hitbox_spaceship, spaceship.radius) && !boss.is_alive && num_pieces == 1)
            win = true;

        if (ColisaoEsferaEsfera(hitbox_spaceship, spaceship.radius, boss.hitbox, boss.radius))
            gameOver = true;

        camera_position_c = player.position;

        //////////////////////////////////////////////////////////////////////////

        /////////////////// MOVIMENTA��O MONSTROS ////////////////////////////////

        if (lookat_boss == false)
        {
           for (size_t i = 0; i < monster.size(); ++i) {

                if (monster[i].is_alive)
                {

                    // Checa se o jogador se aproximou o suficiente do monstro para que este o note
                    if (length(player.position - monster[i].position) < 30.0f)
                        monster[i].proximo = true;
                    else
                        monster[i].proximo = false;

                    // Se o jogador se aproximar do monstro, este o persegue
                    if (monster[i].proximo)
                    {
                        // Atualiza a rota��o do monstro pra sempre estar olhando pro jogador
                        monster[i].angle = -atan2(monster[i].position.z - player.position.z, monster[i].position.x - player.position.x);

                        if (player.position.x - 1.0f < monster[i].position.x)
                            monster[i].position.x -= monster[i].speed * delta_t;

                        if (player.position.x + 1.0f> monster[i].position.x)
                            monster[i].position.x += monster[i].speed  * delta_t;

                        if (player.position.z - 1.0f < monster[i].position.z)
                            monster[i].position.z -= monster[i].speed  * delta_t;

                        if (player.position.z + 1.0f > monster[i].position.z)
                            monster[i].position.z += monster[i].speed  * delta_t;

                        monster[i].hitbox = monster[i].position;

                    }
                }
            }
        }

        //////////////////////////////////////////////////////////////////////////

        /////////////////// MOVIMENTA��O BOSS ////////////////////////////////////

        if (num_pieces == 1 && boss.lifes > 0 && frame_counter_boss > 0)
        {
            boss.is_alive = true;
            direction = (boss.position - spaceship.position) / norm(boss.position - spaceship.position);
            boss.angle = -atan2(direction.z, direction.x);
            lookat_boss = true;
            init_counter_boss = true;
        }

        if (lookat_boss == false)
        {
           if (num_pieces == 1 && boss.lifes > 0)
           {
                boss.position.x -= 5 * direction.x * delta_t;
                boss.position.z -= 5 * direction.z * delta_t;

                boss.hitbox = boss.position;
           }
        }

        //////////////////////////////////////////////////////////////////////////

        /////////////////// VIEW e MODEL /////////////////////////////////////////

        // Computamos a matriz "View" utilizando os par�metros da c�mera para
        // definir o sistema de coordenadas da c�mera.  Veja slides 2-14, 184-190 e 236-242 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
        glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);

        // Agora computamos a matriz de Proje��o.
        glm::mat4 projection;

        // Note que, no sistema de coordenadas da c�mera, os planos near e far
        // est�o no sentido negativo! Veja slides 176-204 do documento Aula_09_Projecoes.pdf.
        float nearplane = -0.1f;  // Posi��o do "near plane"
        float farplane  = -100.0f; // Posi��o do "far plane"

        // Proje��o Perspectiva.
        // Para defini��o do field of view (FOV), veja slides 205-215 do documento Aula_09_Projecoes.pdf.
        float field_of_view = 3.141592 / 3.0f;
        projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);

        glm::mat4 model = Matrix_Identity(); // Transforma��o identidade de modelagem

        // Enviamos as matrizes "view" e "projection" para a placa de v�deo
        // (GPU). Veja o arquivo "shader_vertex.glsl", onde estas s�o
        // efetivamente aplicadas em todos os pontos.
        glUniformMatrix4fv(g_view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(g_projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

        //////////////////////////////////////////////////////////////////////////

        /////////////////// SKYBOX ///////////////////////////////////////////////

        if (!win && !gameOver)
        {
            model = Matrix_Translate(player.position.x, player.position.y, player.position.z);
            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            glUniform1i(g_object_id_uniform, SKYBOX);
            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
            DrawVirtualObject("the_sphere");
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
        }

        //////////////////////////////////////////////////////////////////////////

        /////////////////// TIROS ////////////////////////////////////////////////

        if (g_LeftMouseButtonPressed && num_shots > 0)
        {
            // Impede que o jogador crie infinitos tiros segurando o bot�o esquerdo
            g_LeftMouseButtonPressed = false;

            // Cria um novo objeto de projectile
            Projectile new_shot;

            // Propriedades do tiro
            new_shot.is_active = true;
            new_shot.position = glm::vec4(player.position.x, player.position.y, player.position.z, 1.0f);
            new_shot.speed = glm::vec4(10 * camera_view_vector.x, 10 * camera_view_vector.y, 10 * camera_view_vector.z, 0.0f);
            shot.push_back(new_shot);

            // Diminui o n�mero de tiros poss�veis
            num_shots--;
            if (num_shots < 0)
                num_shots = 0;
        }

        if (reload)
            num_shots = 6;

        for (size_t i = 0; i < shot.size(); ++i) {

            if (shot[i].is_active)
            {

                // Atualiza posi��o do tiro
                shot[i].position += shot[i].speed * delta_t;

                // Se o tiro percorreu mais de 60 unidades de dist�ncia ou colidiu com um monstro, ele some
                if (length(player.position - shot[i].position) > 60.0f)
                    shot[i].is_active = false;

                for (size_t j = 0; j < monster.size(); j++)
                {
                    if (ColisaoEsferaEsfera(shot[i].position, shot[i].radius, monster[j].hitbox, monster[j].radius))
                    {
                        shot[i].is_active = false;
                        monster[j].lifes -= player.damage;
                        if (monster[j].lifes == 0){
                            monster[j].is_alive = false;
                            player.points += 50;
                        }
                        if (monster[j].lifes < 0 && monster[j].is_alive)
                        {
                            monster[j].is_alive = false;
                            monster[j].lifes = 0;
                            player.points += 50;
                        }
                    }
                }

                if (ColisaoEsferaEsfera(shot[i].position, shot[i].radius, boss.position, boss.radius))
                {
                    shot[i].is_active = false;
                    boss.lifes -= player.damage;
                    cout << boss.lifes;
                    if (boss.lifes <= 0)
                    {
                        boss.is_alive = false;
                        boss.lifes = 0;
                    }
                }

                // Desenha o tiro ap�s feita a atualiza��o
                model = Matrix_Translate(shot[i].position.x, shot[i].position.y, shot[i].position.z)
                      * Matrix_Scale(0.025f, 0.025f, 0.025f);
                glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                glUniform1i(g_object_id_uniform, BULLETS);
                DrawVirtualObject("the_sphere");

            }
        }

        //////////////////////////////////////////////////////////////////////////

        /////////////////// COELHO ///////////////////////////////////////////////

        for (int i=0; i < 2; i++)
        {
            if ( i==0 )
            {
                model = Matrix_Translate(bunny_position.x, bunny_position.y, bunny_position.z)
                      * Matrix_Rotate_Z(g_AngleZ)
                      * Matrix_Rotate_Y(g_AngleY)
                      * Matrix_Rotate_X(g_AngleX);
                glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                glUniform1i(g_object_id_uniform, BUNNY);
                if (bunny_alive)
                    DrawVirtualObject("the_bunny");
            }
            else
            {
                model = Matrix_Translate(-500.0f, -100.0f, -500.0f);
                glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                glUniform1i(g_object_id_uniform, BUNNY);
                DrawVirtualObject("the_bunny");

            }
        }

        //////////////////////////////////////////////////////////////////////////

        /////////////////// PLANO ////////////////////////////////////////////////

        model = Matrix_Translate(plane_position.x, plane_position.y, plane_position.z)
              * Matrix_Scale(100.0f, 1.0f, 100.0f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, PLANE);
        DrawVirtualObject("the_plane");

        //////////////////////////////////////////////////////////////////////////

        /////////////////// LIBERDADE ////////////////////////////////////////////

        model = Matrix_Translate(statue_position.x, statue_position.y, statue_position.z)
              * Matrix_Scale(10.0f, 10.0f, 10.0f)
              * Matrix_Rotate_Y(3.141592f*0.75f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, LIBERTY);
        DrawVirtualObject("the_liberty");

        //////////////////////////////////////////////////////////////////////////

        /////////////////// MONSTRO //////////////////////////////////////////////

        for (size_t i = 0; i < monster.size(); ++i) {

            if (monster[i].is_alive)
            {
                model = Matrix_Translate(monster[i].position.x, monster[i].position.y, monster[i].position.z)
                      * Matrix_Scale(2.0f, 2.0f, 2.0f)
                      * Matrix_Rotate_Y(monster[i].angle);
                glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                glUniform1i(g_object_id_uniform, MONSTER);
                if (monster[i].lifes > 0)
                    DrawVirtualObject("the_monster");
            }

        }

        //////////////////////////////////////////////////////////////////////////

        /////////////////// PEDRAS ///////////////////////////////////////////////

        for (int i=0; i<4; i++)
        {
            if (i == 0 || i == 1)
            {
                model = Matrix_Translate(98.0f*pow(-1, i), 0.0f + (0.6*i), 98.0f*pow(-1, i+1))
                      * Matrix_Scale(2.0f + i, 2.0f + i, 2.0f + i)
                      * Matrix_Rotate_Y((3.141592f/2)*(i+1));
            }
            else
            {
                model = Matrix_Translate(98.0f*pow(-1, i), 0.0f + (0.6*i), 98.0f*pow(-1, i))
                      * Matrix_Scale(2.0f + i, 2.0f + i, 2.0f + i)
                      * Matrix_Rotate_Y((3.141592f/2)*(i+1));
            }
            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            glUniform1i(g_object_id_uniform, ROCK);
            DrawVirtualObject("the_rock");
        }

        //////////////////////////////////////////////////////////////////////////

        /////////////////// NAVE /////////////////////////////////////////////////

        model = Matrix_Translate(spaceship.position.x, spaceship.position.y, spaceship.position.z)
              * Matrix_Scale(5.0f, 5.0f, 5.0f)
              * Matrix_Rotate_Y(3.141592f*0.75f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, SPACESHIP);
        DrawVirtualObject("the_ship");

        //////////////////////////////////////////////////////////////////////////

        /////////////////// MORRO ////////////////////////////////////////////////

        model = Matrix_Translate(mount_position.x, mount_position.y, mount_position.z)
              * Matrix_Scale(20.0f, 20.0f, 20.0f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, MOUNT);
        DrawVirtualObject("the_mount");

        //////////////////////////////////////////////////////////////////////////

        /////////////////// PEDA�O DA NAVE ///////////////////////////////////////

        for(int i = 0; i < 5; i++){
            if(piece[i].collected == false){
                model = Matrix_Translate(piece[i].position.x, piece[i].position.y, piece[i].position.z)
                      * Matrix_Scale(0.4f, 0.4f, 0.4f)
                      * Matrix_Rotate_Y(fmod(prev_time, piece[i].angle));
                glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                glUniform1i(g_object_id_uniform, PIECE);
                DrawVirtualObject("the_piece");
            }

        }

        //////////////////////////////////////////////////////////////////////////

        /////////////////// CAPSULAS /////////////////////////////////////////////

        for (int i = 0; i < 3; i++){
            model = Matrix_Translate(capsule[i].position.x, capsule[i].position.y, capsule[i].position.z)
                  * Matrix_Scale(capsule[i].radius, capsule[i].radius, capsule[i].radius)
                  * Matrix_Rotate_Y(fmod(prev_time, capsule[i].angle));
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, CAPSULE);
        DrawVirtualObject("the_capsule");
        DrawVirtualObject("Cylinder.011_Cylinder.022");
        DrawVirtualObject("Cylinder.008_Cylinder.021");
        DrawVirtualObject("Cylinder.005_Cylinder.010");
        DrawVirtualObject("Cylinder.004_Cylinder.009");
        }


        //////////////////////////////////////////////////////////////////////////

        /////////////////// ARVORES ///////////////////////////////////////////////

        model = Matrix_Translate(tree_position.x, tree_position.y, tree_position.z)
              * Matrix_Scale(10.0f, 10.0f, 10.0f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, TREE);
        DrawVirtualObject("the_tree");

        //////////////////////////////////////////////////////////////////////////

        /////////////////// BOSS /////////////////////////////////////////////////

        if(boss.is_alive == true) {
            model = Matrix_Translate(boss.position.x, boss.position.y, boss.position.z)
              * Matrix_Scale(10.0f, 10.0f, 10.0f)
              * Matrix_Rotate_Y(boss.angle);
            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            glUniform1i(g_object_id_uniform, BOSS);
            DrawVirtualObject("the_boss");
        }

        //////////////////////////////////////////////////////////////////////////

        /////////////////// ARMA /////////////////////////////////////////////////

        model = Matrix_Translate(0.06f, -0.115f, -0.2f)
              * Matrix_Scale(0.1f, 0.1f, 0.1f)
              * Matrix_Rotate_Y(M_PI);
        view = Matrix_Identity();
        glUniformMatrix4fv(g_view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, GUN);
        DrawVirtualObject("the_gun");
        view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);
        glUniformMatrix4fv(g_view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));


        //////////////////////////////////////////////////////////////////////////

        /////////////////// BEZIER ///////////////////////////////////////////////

        // Calcula a posi��o atual do monstro na curva de Bezier
        if (ciclo_voo)
            fly_position = CalculaBezier(t, ponto_controle_1, ponto_controle_2, ponto_controle_3, ponto_controle_4);
        else
            fly_position = CalculaBezier(t, ponto_controle_5, ponto_controle_6, ponto_controle_7, ponto_controle_8);

        fly_monster_angle = -atan2(fly_position.z - player.position.z, fly_position.x - player.position.x);

        model = Matrix_Translate(fly_position.x, fly_position.y, fly_position.z)
              * Matrix_Rotate_Y(fly_monster_angle);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, FLYMONSTER);
        DrawVirtualObject("the_flymonster");

        // Incremento para que o objeto se mova na curva
        t += 0.008f;

        // Resete o par�metro de interpola��o para reiniciar o movimento ao completar a curva
        if (t > 1.0f)
        {
            t = 0.0f;
            ciclo_voo = !ciclo_voo;
        }

        //////////////////////////////////////////////////////////////////////////

        /////////////////// HITBOXES /////////////////////////////////////////////

        for (size_t i = 0; i < monster.size(); ++i)
        {
            if (monster[i].is_alive)
            {
                // Esfera de colis�o centrada nos monstros
                model = Matrix_Translate(monster[i].hitbox.x, monster[i].hitbox.y, monster[i].hitbox.z)
                      * Matrix_Scale(1.5f, 1.5f, 1.5f);
                glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                glUniform1i(g_object_id_uniform, HITBOX);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                DrawVirtualObject("the_sphere");
                glDisable(GL_BLEND);
            }
        }

        // Esfera de colis�o centrada no coelho
        model = Matrix_Translate(hitbox_bunny.x, hitbox_bunny.y, hitbox_bunny.z)
              * Matrix_Scale(2.0f, 2.0f, 2.0f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, HITBOX);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        DrawVirtualObject("the_sphere");
        glDisable(GL_BLEND);

        // Esfera de colis�o centrada na nave
        model = Matrix_Translate(hitbox_spaceship.x, hitbox_spaceship.y, hitbox_spaceship.z)
              * Matrix_Scale(2.0f, 2.0f, 2.0f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, HITBOX);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        DrawVirtualObject("the_sphere");
        glDisable(GL_BLEND);

        // Esfera de colis�o centrada no boss
        if(boss.is_alive == true){
            model = Matrix_Translate(boss.hitbox.x, boss.hitbox.y, boss.hitbox.z)
              * Matrix_Scale(11.0f, 11.0f, 11.0f);
            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            glUniform1i(g_object_id_uniform, HITBOX);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            DrawVirtualObject("the_sphere");
            glDisable(GL_BLEND);
        }

        // Esfera de colis�o centrada nas capsulas
        for (size_t i = 0; i < capsule.size(); ++i)
        {
            model = Matrix_Translate(capsule[i].hitbox.x, capsule[i].hitbox.y, capsule[i].hitbox.z)
                  * Matrix_Scale(1.0f, 1.0f, 1.0f);
            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            glUniform1i(g_object_id_uniform, HITBOX);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            DrawVirtualObject("the_sphere");
            glDisable(GL_BLEND);
        }

        //////////////////////////////////////////////////////////////////////////

        if (player.is_alive == true && !gameOver && !win)
        {
            // Imprimimos na tela a quantidade de tiros que o jogador possui
            TextRendering_ShowBullets(window);

            // Imprimimos na tela a quantidade de vidas que o jogador possui
            TextRendering_ShowLifes(window);

            // Imprimimos na tela a quantidade de pe�as da nave que o jogador possui
            TextRendering_ShowPieces(window);

            // Imprimimos na tela informa��o sobre o n�mero de quadros renderizados
            // por segundo (frames per second).
            TextRendering_ShowFramesPerSecond(window);

            // Imprimimos na tela a quantidade de tempo restante at� que o oxig�nio acabe
            TextRendering_ShowTime(window);

            // Imprimimos na tela a quantidade de pontos que o jogador possui
            TextRendering_ShowPoints(window, player.points);

            // Imprimimos na tela a mensagem de que o player pode comprar um upgrade
            if(capsule[0].colide || capsule[1].colide || capsule[2].colide)
            {
                TextRendering_ShowBuyUpgrade(window, price);
            }

            // Imprimimos na tela a mensagem de que uma vida extra foi adquirida
            if(show_message_1)
            {
                TextRendering_ShowMessageExtraLife(window);
                if((float)glfwGetTime() >= upgrade_massage_time + 2.0f)
                {
                    show_message_1 = false;
                    canBuy = true;
                }
            }

            // Imprimimos na tela a mensagem de que o dano foi aumentado
            if(show_message_2)
            {
                TextRendering_ShowMessageIncDamage(window);
                if((float)glfwGetTime() >= upgrade_massage_time + 2.0f)
                {
                    show_message_2 = false;
                    canBuy = true;
                }
            }

            // Imprimimos na tela a mensagem de que a velocidade de movimento foi aumentada
            if(show_message_3)
            {
                TextRendering_ShowMessageIncSpeed(window);
                if((float)glfwGetTime() >= upgrade_massage_time + 2.0f)
                {
                    show_message_3 = false;
                    canBuy = true;
                }
            }

            // Imprimimos na tela a mensagem de que o player n�o possui pontos o suficiente para adquirir um upgrade
            if(show_message_4)
            {
                TextRendering_ShowMessageInsufficientPoints(window);
                if((float)glfwGetTime() >= upgrade_massage_time + 2.0f)
                {
                    show_message_4 = false;
                    canBuy = true;
                }
            }

            // desenha a crossair na frente da tela
            glDisable(GL_DEPTH_TEST);
            glBindVertexArray(vertex_array_object_id_crosshair);
            glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_BYTE, 0);
            glEnable(GL_DEPTH_TEST);

        }

        if(gameOver)
        {
            // Imprimimos na tela a mensagem de fim de jogo
            TextRendering_ShowGameOver(window);
        }

        if(win)
        {
            // Imprimimos na tela a mensagem de fim de jogo
            TextRendering_ShowWin(window);
        }

        // Desligamos o VAO
        glBindVertexArray(0);

        // Atualiza delta de tempo
        float current_time = (float)glfwGetTime();
        delta_t = current_time - prev_time;
        prev_time = current_time;

        if (init_counter_boss)
            frame_counter_boss--;
        if (frame_counter_boss <= 0)
            init_counter_boss = false;

        // O framebuffer onde OpenGL executa as opera��es de renderiza��o n�o
        // � o mesmo que est� sendo mostrado para o usu�rio, caso contr�rio
        // seria poss�vel ver artefatos conhecidos como "screen tearing". A
        // chamada abaixo faz a troca dos buffers, mostrando para o usu�rio
        // tudo que foi renderizado pelas fun��es acima.
        // Veja o link: https://en.wikipedia.org/w/index.php?title=Multiple_buffering&oldid=793452829#Double_buffering_in_computer_graphics
        glfwSwapBuffers(window);

        // Verificamos com o sistema operacional se houve alguma intera��o do
        // usu�rio (teclado, mouse, ...). Caso positivo, as fun��es de callback
        // definidas anteriormente usando glfwSet*Callback() ser�o chamadas
        // pela biblioteca GLFW.
        glfwPollEvents();
    }

    // Finalizamos o uso dos recursos do sistema operacional
    glfwTerminate();

    // Fim do programa
    return 0;
}

// Fun��o que carrega uma imagem para ser utilizada como textura
void LoadTextureImage(const char* filename)
{
    printf("Carregando imagem \"%s\"... ", filename);

    // Primeiro fazemos a leitura da imagem do disco
    stbi_set_flip_vertically_on_load(true);
    int width;
    int height;
    int channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 3);

    if ( data == NULL )
    {
        fprintf(stderr, "ERROR: Cannot open image file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }

    printf("OK (%dx%d).\n", width, height);

    // Agora criamos objetos na GPU com OpenGL para armazenar a textura
    GLuint texture_id;
    GLuint sampler_id;
    glGenTextures(1, &texture_id);
    glGenSamplers(1, &sampler_id);

    // Veja slides 95-96 do documento Aula_20_Mapeamento_de_Texturas.pdf
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    // Par�metros de amostragem da textura.
    glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Agora enviamos a imagem lida do disco para a GPU
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    GLuint textureunit = g_NumLoadedTextures;
    glActiveTexture(GL_TEXTURE0 + textureunit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindSampler(textureunit, sampler_id);

    stbi_image_free(data);

    g_NumLoadedTextures += 1;
}

// Fun��o que desenha um objeto armazenado em g_VirtualScene. Veja defini��o
// dos objetos na fun��o BuildTrianglesAndAddToVirtualScene().
void DrawVirtualObject(const char* object_name)
{
    // "Ligamos" o VAO. Informamos que queremos utilizar os atributos de
    // v�rtices apontados pelo VAO criado pela fun��o BuildTrianglesAndAddToVirtualScene(). Veja
    // coment�rios detalhados dentro da defini��o de BuildTrianglesAndAddToVirtualScene().
    glBindVertexArray(g_VirtualScene[object_name].vertex_array_object_id);

    // Setamos as vari�veis "bbox_min" e "bbox_max" do fragment shader
    // com os par�metros da axis-aligned bounding box (AABB) do modelo.
    glm::vec3 bbox_min = g_VirtualScene[object_name].bbox_min;
    glm::vec3 bbox_max = g_VirtualScene[object_name].bbox_max;
    glUniform4f(g_bbox_min_uniform, bbox_min.x, bbox_min.y, bbox_min.z, 1.0f);
    glUniform4f(g_bbox_max_uniform, bbox_max.x, bbox_max.y, bbox_max.z, 1.0f);

    // Pedimos para a GPU rasterizar os v�rtices dos eixos XYZ
    // apontados pelo VAO como linhas. Veja a defini��o de
    // g_VirtualScene[""] dentro da fun��o BuildTrianglesAndAddToVirtualScene(), e veja
    // a documenta��o da fun��o glDrawElements() em
    // http://docs.gl/gl3/glDrawElements.
    glDrawElements(
        g_VirtualScene[object_name].rendering_mode,
        g_VirtualScene[object_name].num_indices,
        GL_UNSIGNED_INT,
        (void*)(g_VirtualScene[object_name].first_index * sizeof(GLuint))
    );

    // "Desligamos" o VAO, evitando assim que opera��es posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Constr�i tri�ngulos para futura renderiza��o
GLuint BuildTrianglesForCrosshair()
{

    GLfloat NDC_coefficients[] = {
        -0.025f,  0.00625f, 0.0f, 1.0f, // posi��o do v�rtice 0
         0.025f,  0.00625f, 0.0f, 1.0f, // posi��o do v�rtice 1
         0.025f, -0.00625f, 0.0f, 1.0f, // posi��o do v�rtice 2
        -0.025f, -0.00625f, 0.0f, 1.0f, // posi��o do v�rtice 3

        -0.00625f,  0.025f, 0.0f, 1.0f, // posi��o do v�rtice 4
         0.00625f,  0.025f, 0.0f, 1.0f, // posi��o do v�rtice 5
         0.00625f, -0.025f, 0.0f, 1.0f, // posi��o do v�rtice 6
        -0.00625f, -0.025f, 0.0f, 1.0f, // posi��o do v�rtice 7
    };

    GLuint VBO_NDC_coefficients_id;
    glGenBuffers(1, &VBO_NDC_coefficients_id);

    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);

    glBindVertexArray(vertex_array_object_id);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_NDC_coefficients_id);

    glBufferData(GL_ARRAY_BUFFER, sizeof(NDC_coefficients), NULL, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(NDC_coefficients), NDC_coefficients);

    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(location);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLfloat color_coefficients[32] = {};

    for (int i=0; i<32; i+=4){
        color_coefficients[i] = .0f;
        color_coefficients[i+1] = .0f;
        color_coefficients[i+2] = .0f;
        color_coefficients[i+3] = 1.0f;
    };

    GLuint VBO_color_coefficients_id;
    glGenBuffers(1, &VBO_color_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_color_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_coefficients), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(color_coefficients), color_coefficients);
    location = 1; // "(location = 1)" em "shader_vertex.glsl"
    number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    index_type indices[]={3, 1, 0,
                          3, 2, 1,
                          6, 5, 4,
                          6, 4, 7};

    // Criamos um buffer OpenGL para armazenar os �ndices acima
    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora � GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);

    // Alocamos mem�ria para o buffer.
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), NULL, GL_STATIC_DRAW);

    // Copiamos os valores do array indices[] para dentro do buffer.
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);

    glBindVertexArray(0);

    return vertex_array_object_id;
}

GLuint BuildTrianglesForGameOverScreen()
{

    GLfloat NDC_coefficients[] = {
        -1.0f,  -1.0f, 0.0f, 1.0f, // posi��o do v�rtice 0
        -1.0f,   1.0f, 0.0f, 1.0f, // posi��o do v�rtice 1
         1.0f,   1.0f, 0.0f, 1.0f, // posi��o do v�rtice 2
         1.0f,  -1.0f, 0.0f, 1.0f, // posi��o do v�rtice 3

    };

    GLuint VBO_NDC_coefficients_id;
    glGenBuffers(1, &VBO_NDC_coefficients_id);

    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);

    glBindVertexArray(vertex_array_object_id);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_NDC_coefficients_id);

    glBufferData(GL_ARRAY_BUFFER, sizeof(NDC_coefficients), NULL, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(NDC_coefficients), NDC_coefficients);

    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(location);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLfloat color_coefficients[16] = {};

    for (int i=0; i<16; i+=4){
        color_coefficients[i] = 1.0f;
        color_coefficients[i+1] = 1.0f;
        color_coefficients[i+2] = 1.0f;
        color_coefficients[i+3] = 1.0f;
    };

    GLuint VBO_color_coefficients_id;
    glGenBuffers(1, &VBO_color_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_color_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_coefficients), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(color_coefficients), color_coefficients);
    location = 1; // "(location = 1)" em "shader_vertex.glsl"
    number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    index_type indices[]={3, 1, 0,
                          3, 2, 1};

    // Criamos um buffer OpenGL para armazenar os �ndices acima
    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora � GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);

    // Alocamos mem�ria para o buffer.
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), NULL, GL_STATIC_DRAW);

    // Copiamos os valores do array indices[] para dentro do buffer.
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);

    glBindVertexArray(0);

    return vertex_array_object_id;
}

// Fun��o que carrega os shaders de v�rtices e de fragmentos que ser�o
// utilizados para renderiza��o. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
//
void LoadShadersFromFiles()
{
    // Note que o caminho para os arquivos "shader_vertex.glsl" e
    // "shader_fragment.glsl" est�o fixados, sendo que assumimos a exist�ncia
    // da seguinte estrutura no sistema de arquivos:
    //
    //    + FCG_Lab_01/
    //    |
    //    +--+ bin/
    //    |  |
    //    |  +--+ Release/  (ou Debug/ ou Linux/)
    //    |     |
    //    |     o-- main.exe
    //    |
    //    +--+ src/
    //       |
    //       o-- shader_vertex.glsl
    //       |
    //       o-- shader_fragment.glsl
    //
    GLuint vertex_shader_id = LoadShader_Vertex("../../src/shader_vertex.glsl");
    GLuint fragment_shader_id = LoadShader_Fragment("../../src/shader_fragment.glsl");

    // Deletamos o programa de GPU anterior, caso ele exista.
    if ( g_GpuProgramID != 0 )
        glDeleteProgram(g_GpuProgramID);

    // Criamos um programa de GPU utilizando os shaders carregados acima.
    g_GpuProgramID = CreateGpuProgram(vertex_shader_id, fragment_shader_id);

    // Buscamos o endere�o das vari�veis definidas dentro do Vertex Shader.
    // Utilizaremos estas vari�veis para enviar dados para a placa de v�deo
    // (GPU)! Veja arquivo "shader_vertex.glsl" e "shader_fragment.glsl".
    g_model_uniform      = glGetUniformLocation(g_GpuProgramID, "model"); // Vari�vel da matriz "model"
    g_view_uniform       = glGetUniformLocation(g_GpuProgramID, "view"); // Vari�vel da matriz "view" em shader_vertex.glsl
    g_projection_uniform = glGetUniformLocation(g_GpuProgramID, "projection"); // Vari�vel da matriz "projection" em shader_vertex.glsl
    g_object_id_uniform  = glGetUniformLocation(g_GpuProgramID, "object_id"); // Vari�vel "object_id" em shader_fragment.glsl
    g_bbox_min_uniform   = glGetUniformLocation(g_GpuProgramID, "bbox_min");
    g_bbox_max_uniform   = glGetUniformLocation(g_GpuProgramID, "bbox_max");

    // Vari�veis em "shader_fragment.glsl" para acesso das imagens de textura
    glUseProgram(g_GpuProgramID);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage0"), 0);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage1"), 1);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage2"), 2);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage3"), 3);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage4"), 4);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage5"), 5);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage6"), 6);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage7"), 7);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage8"), 8);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage9"), 9);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage10"), 10);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage11"), 11);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage12"), 12);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage13"), 13);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage14"), 14);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage15"), 15);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage16"), 16);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage17"), 17);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage18"), 18);

    glUseProgram(0);
}

// Fun��o que pega a matriz M e guarda a mesma no topo da pilha
void PushMatrix(glm::mat4 M)
{
    g_MatrixStack.push(M);
}

// Fun��o que remove a matriz atualmente no topo da pilha e armazena a mesma na vari�vel M
void PopMatrix(glm::mat4& M)
{
    if ( g_MatrixStack.empty() )
    {
        M = Matrix_Identity();
    }
    else
    {
        M = g_MatrixStack.top();
        g_MatrixStack.pop();
    }
}

// Fun��o que computa as normais de um ObjModel, caso elas n�o tenham sido
// especificadas dentro do arquivo ".obj"
void ComputeNormals(ObjModel* model)
{
    if ( !model->attrib.normals.empty() )
        return;

    // Primeiro computamos as normais para todos os TRI�NGULOS.
    // Segundo, computamos as normais dos V�RTICES atrav�s do m�todo proposto
    // por Gouraud, onde a normal de cada v�rtice vai ser a m�dia das normais de
    // todas as faces que compartilham este v�rtice.

    size_t num_vertices = model->attrib.vertices.size() / 3;

    std::vector<int> num_triangles_per_vertex(num_vertices, 0);
    std::vector<glm::vec4> vertex_normals(num_vertices, glm::vec4(0.0f,0.0f,0.0f,0.0f));

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            glm::vec4  vertices[3];
            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                vertices[vertex] = glm::vec4(vx,vy,vz,1.0);
            }

            const glm::vec4  a = vertices[0];
            const glm::vec4  b = vertices[1];
            const glm::vec4  c = vertices[2];

            const glm::vec4  n = crossproduct(b-a, c-a);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                num_triangles_per_vertex[idx.vertex_index] += 1;
                vertex_normals[idx.vertex_index] += n;
                model->shapes[shape].mesh.indices[3*triangle + vertex].normal_index = idx.vertex_index;
            }
        }
    }

    model->attrib.normals.resize( 3*num_vertices );

    for (size_t i = 0; i < vertex_normals.size(); ++i)
    {
        glm::vec4 n = vertex_normals[i] / (float)num_triangles_per_vertex[i];
        n /= norm(n);
        model->attrib.normals[3*i + 0] = n.x;
        model->attrib.normals[3*i + 1] = n.y;
        model->attrib.normals[3*i + 2] = n.z;
    }
}

// Constr�i tri�ngulos para futura renderiza��o a partir de um ObjModel.
void BuildTrianglesAndAddToVirtualScene(ObjModel* model)
{
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);

    std::vector<GLuint> indices;
    std::vector<float>  model_coefficients;
    std::vector<float>  normal_coefficients;
    std::vector<float>  texture_coefficients;

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t first_index = indices.size();
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        const float minval = std::numeric_limits<float>::min();
        const float maxval = std::numeric_limits<float>::max();

        glm::vec3 bbox_min = glm::vec3(maxval,maxval,maxval);
        glm::vec3 bbox_max = glm::vec3(minval,minval,minval);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];

                indices.push_back(first_index + 3*triangle + vertex);

                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                //printf("tri %d vert %d = (%.2f, %.2f, %.2f)\n", (int)triangle, (int)vertex, vx, vy, vz);
                model_coefficients.push_back( vx ); // X
                model_coefficients.push_back( vy ); // Y
                model_coefficients.push_back( vz ); // Z
                model_coefficients.push_back( 1.0f ); // W

                bbox_min.x = std::min(bbox_min.x, vx);
                bbox_min.y = std::min(bbox_min.y, vy);
                bbox_min.z = std::min(bbox_min.z, vz);
                bbox_max.x = std::max(bbox_max.x, vx);
                bbox_max.y = std::max(bbox_max.y, vy);
                bbox_max.z = std::max(bbox_max.z, vz);

                // Inspecionando o c�digo da tinyobjloader, o aluno Bernardo
                // Sulzbach (2017/1) apontou que a maneira correta de testar se
                // existem normais e coordenadas de textura no ObjModel �
                // comparando se o �ndice retornado � -1. Fazemos isso abaixo.

                if ( idx.normal_index != -1 )
                {
                    const float nx = model->attrib.normals[3*idx.normal_index + 0];
                    const float ny = model->attrib.normals[3*idx.normal_index + 1];
                    const float nz = model->attrib.normals[3*idx.normal_index + 2];
                    normal_coefficients.push_back( nx ); // X
                    normal_coefficients.push_back( ny ); // Y
                    normal_coefficients.push_back( nz ); // Z
                    normal_coefficients.push_back( 0.0f ); // W
                }

                if ( idx.texcoord_index != -1 )
                {
                    const float u = model->attrib.texcoords[2*idx.texcoord_index + 0];
                    const float v = model->attrib.texcoords[2*idx.texcoord_index + 1];
                    texture_coefficients.push_back( u );
                    texture_coefficients.push_back( v );
                }
            }
        }

        size_t last_index = indices.size() - 1;

        SceneObject theobject;
        theobject.name           = model->shapes[shape].name;
        theobject.first_index    = first_index; // Primeiro �ndice
        theobject.num_indices    = last_index - first_index + 1; // N�mero de indices
        theobject.rendering_mode = GL_TRIANGLES;       // �ndices correspondem ao tipo de rasteriza��o GL_TRIANGLES.
        theobject.vertex_array_object_id = vertex_array_object_id;

        theobject.bbox_min = bbox_min;
        theobject.bbox_max = bbox_max;

        g_VirtualScene[model->shapes[shape].name] = theobject;
    }

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if ( !normal_coefficients.empty() )
    {
        GLuint VBO_normal_coefficients_id;
        glGenBuffers(1, &VBO_normal_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
        location = 1; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if ( !texture_coefficients.empty() )
    {
        GLuint VBO_texture_coefficients_id;
        glGenBuffers(1, &VBO_texture_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, texture_coefficients.size() * sizeof(float), texture_coefficients.data());
        location = 2; // "(location = 2)" em "shader_vertex.glsl"
        number_of_dimensions = 2; // vec2 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

    }

    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora � GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
    //

    // "Desligamos" o VAO, evitando assim que opera��es posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Carrega um Vertex Shader de um arquivo GLSL. Veja defini��o de LoadShader() abaixo.
GLuint LoadShader_Vertex(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // ser� aplicado nos v�rtices.
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, vertex_shader_id);

    // Retorna o ID gerado acima
    return vertex_shader_id;
}

// Carrega um Fragment Shader de um arquivo GLSL . Veja defini��o de LoadShader() abaixo.
GLuint LoadShader_Fragment(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // ser� aplicado nos fragmentos.
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, fragment_shader_id);

    // Retorna o ID gerado acima
    return fragment_shader_id;
}

// Fun��o auxilar, utilizada pelas duas fun��es acima. Carrega c�digo de GPU de
// um arquivo GLSL e faz sua compila��o.
void LoadShader(const char* filename, GLuint shader_id)
{
    // Lemos o arquivo de texto indicado pela vari�vel "filename"
    // e colocamos seu conte�do em mem�ria, apontado pela vari�vel
    // "shader_string".
    std::ifstream file;
    try {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    } catch ( std::exception& e ) {
        fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream shader;
    shader << file.rdbuf();
    std::string str = shader.str();
    const GLchar* shader_string = str.c_str();
    const GLint   shader_string_length = static_cast<GLint>( str.length() );

    // Define o c�digo do shader GLSL, contido na string "shader_string"
    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);

    // Compila o c�digo do shader GLSL (em tempo de execu��o)
    glCompileShader(shader_id);

    // Verificamos se ocorreu algum erro ou "warning" durante a compila��o
    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);

    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

    // Alocamos mem�ria para guardar o log de compila��o.
    // A chamada "new" em C++ � equivalente ao "malloc()" do C.
    GLchar* log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);

    // Imprime no terminal qualquer erro ou "warning" de compila��o
    if ( log_length != 0 )
    {
        std::string  output;

        if ( !compiled_ok )
        {
            output += "ERROR: OpenGL compilation of \"";
            output += filename;
            output += "\" failed.\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }
        else
        {
            output += "WARNING: OpenGL compilation of \"";
            output += filename;
            output += "\".\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }

        fprintf(stderr, "%s", output.c_str());
    }

    // A chamada "delete" em C++ � equivalente ao "free()" do C
    delete [] log;
}

// Esta fun��o cria um programa de GPU, o qual cont�m obrigatoriamente um
// Vertex Shader e um Fragment Shader.
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
    // Criamos um identificador (ID) para este programa de GPU
    GLuint program_id = glCreateProgram();

    // Defini��o dos dois shaders GLSL que devem ser executados pelo programa
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);

    // Linkagem dos shaders acima ao programa
    glLinkProgram(program_id);

    // Verificamos se ocorreu algum erro durante a linkagem
    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);

    // Imprime no terminal qualquer erro de linkagem
    if ( linked_ok == GL_FALSE )
    {
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

        // Alocamos mem�ria para guardar o log de compila��o.
        // A chamada "new" em C++ � equivalente ao "malloc()" do C.
        GLchar* log = new GLchar[log_length];

        glGetProgramInfoLog(program_id, log_length, &log_length, log);

        std::string output;

        output += "ERROR: OpenGL linking of program failed.\n";
        output += "== Start of link log\n";
        output += log;
        output += "\n== End of link log\n";

        // A chamada "delete" em C++ � equivalente ao "free()" do C
        delete [] log;

        fprintf(stderr, "%s", output.c_str());
    }

    // Os "Shader Objects" podem ser marcados para dele��o ap�s serem linkados
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    // Retornamos o ID gerado acima
    return program_id;
}

// Defini��o da fun��o que ser� chamada sempre que a janela do sistema
// operacional for redimensionada, por consequ�ncia alterando o tamanho do
// "framebuffer" (regi�o de mem�ria onde s�o armazenados os pixels da imagem).
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // Indicamos que queremos renderizar em toda regi�o do framebuffer. A
    // fun��o "glViewport" define o mapeamento das "normalized device
    // coordinates" (NDC) para "pixel coordinates".  Essa � a opera��o de
    // "Screen Mapping" ou "Viewport Mapping" vista em aula ({+ViewportMapping2+}).
    glViewport(0, 0, width, height);

    // Atualizamos tamb�m a raz�o que define a propor��o da janela (largura /
    // altura), a qual ser� utilizada na defini��o das matrizes de proje��o,
    // tal que n�o ocorra distor��es durante o processo de "Screen Mapping"
    // acima, quando NDC � mapeado para coordenadas de pixels. Veja slides 205-215 do documento Aula_09_Projecoes.pdf.
    //
    // O cast para float � necess�rio pois n�meros inteiros s�o arredondados ao
    // serem divididos!
    g_ScreenRatio = (float)width / height;
}

// Vari�veis globais que armazenam a �ltima posi��o do cursor do mouse, para
// que possamos calcular quanto que o mouse se movimentou entre dois instantes
// de tempo. Utilizadas no callback CursorPosCallback() abaixo.
double g_LastCursorPosX, g_LastCursorPosY;

// Fun��o callback chamada sempre que o usu�rio aperta algum dos bot�es do mouse
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Se o usu�rio pressionou o bot�o esquerdo do mouse, guardamos a
        // posi��o atual do cursor nas vari�veis g_LastCursorPosX e
        // g_LastCursorPosY.  Tamb�m, setamos a vari�vel
        // g_LeftMouseButtonPressed como true, para saber que o usu�rio est�
        // com o bot�o esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_LeftMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        // Quando o usu�rio soltar o bot�o esquerdo do mouse, atualizamos a
        // vari�vel abaixo para false.
        g_LeftMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        // Se o usu�rio pressionou o bot�o esquerdo do mouse, guardamos a
        // posi��o atual do cursor nas vari�veis g_LastCursorPosX e
        // g_LastCursorPosY.  Tamb�m, setamos a vari�vel
        // g_RightMouseButtonPressed como true, para saber que o usu�rio est�
        // com o bot�o esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_RightMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        // Quando o usu�rio soltar o bot�o esquerdo do mouse, atualizamos a
        // vari�vel abaixo para false.
        g_RightMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    {
        // Se o usu�rio pressionou o bot�o esquerdo do mouse, guardamos a
        // posi��o atual do cursor nas vari�veis g_LastCursorPosX e
        // g_LastCursorPosY.  Tamb�m, setamos a vari�vel
        // g_MiddleMouseButtonPressed como true, para saber que o usu�rio est�
        // com o bot�o esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_MiddleMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
    {
        // Quando o usu�rio soltar o bot�o esquerdo do mouse, atualizamos a
        // vari�vel abaixo para false.
        g_MiddleMouseButtonPressed = false;
    }
}

// Fun��o callback chamada sempre que o usu�rio movimentar o cursor do mouse em
// cima da janela OpenGL.
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (!lookat_boss)
    {
        // C�mera se move junto com o mouse do usu�rio
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;

        // Atualizamos par�metros da c�mera com os deslocamentos
        g_CameraTheta += 0.01f*dx;
        g_CameraPhi   -= 0.01f*dy;

        // Em coordenadas esf�ricas, o �ngulo phi deve ficar entre -pi/2 e +pi/2.
        float phimax = 3.141592f/2;
        float phimin = -phimax;

        if (g_CameraPhi > phimax)
            g_CameraPhi = phimax;
        if (g_CameraPhi < phimin)
            g_CameraPhi = phimin;

        // Atualizamos as vari�veis globais para armazenar a posi��o atual do
        // cursor como sendo a �ltima posi��o conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;

        if (g_LeftMouseButtonPressed)
        {
            // atira
        }

        if (g_RightMouseButtonPressed)
        {
            // mira?
        }

        if (g_MiddleMouseButtonPressed)
        {
            // zoom
        }
    }
}

// Fun��o callback chamada sempre que o usu�rio movimenta a "rodinha" do mouse.
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Atualizamos a dist�ncia da c�mera para a origem utilizando a
    // movimenta��o da "rodinha", simulando um ZOOM.
    g_CameraDistance -= 0.9f*yoffset;

    // Uma c�mera look-at nunca pode estar exatamente "em cima" do ponto para
    // onde ela est� olhando, pois isto gera problemas de divis�o por zero na
    // defini��o do sistema de coordenadas da c�mera. Isto �, a vari�vel abaixo
    // nunca pode ser zero. Vers�es anteriores deste c�digo possu�am este bug,
    // o qual foi detectado pelo aluno Vinicius Fraga (2017/2).
    const float verysmallnumber = std::numeric_limits<float>::epsilon();
    if (g_CameraDistance < verysmallnumber)
        g_CameraDistance = verysmallnumber;
}

// Defini��o da fun��o que ser� chamada sempre que o usu�rio pressionar alguma
// tecla do teclado. Veja http://www.glfw.org/docs/latest/input_guide.html#input_key
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    // ==================
    // N�o modifique este loop! Ele � utilizando para corre��o automatizada dos
    // laborat�rios. Deve ser sempre o primeiro comando desta fun��o KeyCallback().
    for (int i = 0; i < 10; ++i)
        if (key == GLFW_KEY_0 + i && action == GLFW_PRESS && mod == GLFW_MOD_SHIFT)
            std::exit(100 + i);
    // ==================

    // Se o usu�rio pressionar a tecla ESC, fechamos a janela.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // O c�digo abaixo implementa a seguinte l�gica:
    //   Se apertar tecla X       ent�o g_AngleX += delta;
    //   Se apertar tecla shift+X ent�o g_AngleX -= delta;
    //   Se apertar tecla Y       ent�o g_AngleY += delta;
    //   Se apertar tecla shift+Y ent�o g_AngleY -= delta;
    //   Se apertar tecla Z       ent�o g_AngleZ += delta;
    //   Se apertar tecla shift+Z ent�o g_AngleZ -= delta;

    float delta = 3.141592 / 16; // 22.5 graus, em radianos.

    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        g_AngleX += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        g_AngleY += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        g_AngleZ += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

    // Se o usu�rio apertar a tecla espa�o, resetamos os �ngulos de Euler para zero.
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        jump = true;
        g_AngleX = 0.0f;
        g_AngleY = 0.0f;
        g_AngleZ = 0.0f;
    }
    else
        jump = false;

    // Se o usu�rio apertar a tecla P
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        // faz alguma coisa
    }

    // Se o usu�rio apertar a tecla O
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
    {
        // Faz alguma coisa
    }

    // Se o usu�rio apertar a tecla H, fazemos um "toggle" do texto informativo mostrado na tela.
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
    {
        g_ShowInfoText = !g_ShowInfoText;
    }

    // Se o usu�rio apertar a tecla R, recarregamos os shaders dos arquivos "shader_fragment.glsl" e "shader_vertex.glsl".
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        LoadShadersFromFiles();
        // fprintf(stdout,"Shaders recarregados!\n");
        fflush(stdout);
        reload = true;
    }
    else
        reload = false;

    if (key == GLFW_KEY_W)
    {
        if (action == GLFW_PRESS)
            // Usu�rio apertou a tecla D, ent�o atualizamos o estado para pressionada
            tecla_W_pressionada = true;

        else if (action == GLFW_RELEASE)
            // Usu�rio largou a tecla D, ent�o atualizamos o estado para N�O pressionada
            tecla_W_pressionada = false;

        else if (action == GLFW_REPEAT)
            // Usu�rio est� segurando a tecla D e o sistema operacional est�
            // disparando eventos de repeti��o. Neste caso, n�o precisamos
            // atualizar o estado da tecla, pois antes de um evento REPEAT
            // necessariamente deve ter ocorrido um evento PRESS.
            ;
    }

    if (key == GLFW_KEY_A)
    {
        if (action == GLFW_PRESS)
            // Usu�rio apertou a tecla D, ent�o atualizamos o estado para pressionada
            tecla_A_pressionada = true;

        else if (action == GLFW_RELEASE)
            // Usu�rio largou a tecla D, ent�o atualizamos o estado para N�O pressionada
            tecla_A_pressionada = false;

        else if (action == GLFW_REPEAT)
            // Usu�rio est� segurando a tecla D e o sistema operacional est�
            // disparando eventos de repeti��o. Neste caso, n�o precisamos
            // atualizar o estado da tecla, pois antes de um evento REPEAT
            // necessariamente deve ter ocorrido um evento PRESS.
            ;
    }

    if (key == GLFW_KEY_S)
    {
        if (action == GLFW_PRESS)
            // Usu�rio apertou a tecla D, ent�o atualizamos o estado para pressionada
            tecla_S_pressionada = true;

        else if (action == GLFW_RELEASE)
            // Usu�rio largou a tecla D, ent�o atualizamos o estado para N�O pressionada
            tecla_S_pressionada = false;

        else if (action == GLFW_REPEAT)
            // Usu�rio est� segurando a tecla D e o sistema operacional est�
            // disparando eventos de repeti��o. Neste caso, n�o precisamos
            // atualizar o estado da tecla, pois antes de um evento REPEAT
            // necessariamente deve ter ocorrido um evento PRESS.
            ;
    }

    if (key == GLFW_KEY_D)
    {
        if (action == GLFW_PRESS)
            // Usu�rio apertou a tecla D, ent�o atualizamos o estado para pressionada
            tecla_D_pressionada = true;

        else if (action == GLFW_RELEASE)
            // Usu�rio largou a tecla D, ent�o atualizamos o estado para N�O pressionada
            tecla_D_pressionada = false;

        else if (action == GLFW_REPEAT)
            // Usu�rio est� segurando a tecla D e o sistema operacional est�
            // disparando eventos de repeti��o. Neste caso, n�o precisamos
            // atualizar o estado da tecla, pois antes de um evento REPEAT
            // necessariamente deve ter ocorrido um evento PRESS.
            ;
    }

    if (key == GLFW_KEY_E)
    {
        if (action == GLFW_PRESS)
            // Usu�rio apertou a tecla D, ent�o atualizamos o estado para pressionada
            tecla_E_pressionada = true;

        else if (action == GLFW_RELEASE)
            // Usu�rio largou a tecla D, ent�o atualizamos o estado para N�O pressionada
            tecla_E_pressionada = false;
    }

}

// Definimos o callback para impress�o de erros da GLFW no terminal
void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}

// Esta fun��o recebe um v�rtice com coordenadas de modelo p_model e passa o
// mesmo por todos os sistemas de coordenadas armazenados nas matrizes model,
// view, e projection; e escreve na tela as matrizes e pontos resultantes
// dessas transforma��es.
void TextRendering_ShowModelViewProjection(
    GLFWwindow* window,
    glm::mat4 projection,
    glm::mat4 view,
    glm::mat4 model,
    glm::vec4 p_model
)
{
    if ( !g_ShowInfoText )
        return;

    glm::vec4 p_world = model*p_model;
    glm::vec4 p_camera = view*p_world;
    glm::vec4 p_clip = projection*p_camera;
    glm::vec4 p_ndc = p_clip / p_clip.w;

    float pad = TextRendering_LineHeight(window);

    TextRendering_PrintString(window, " Model matrix             Model     In World Coords.", -1.0f, 1.0f-pad, 1.0f);
    TextRendering_PrintMatrixVectorProduct(window, model, p_model, -1.0f, 1.0f-2*pad, 1.0f);

    TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-6*pad, 1.0f);
    TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-7*pad, 1.0f);
    TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-8*pad, 1.0f);

    TextRendering_PrintString(window, " View matrix              World     In Camera Coords.", -1.0f, 1.0f-9*pad, 1.0f);
    TextRendering_PrintMatrixVectorProduct(window, view, p_world, -1.0f, 1.0f-10*pad, 1.0f);

    TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-14*pad, 1.0f);
    TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-15*pad, 1.0f);
    TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-16*pad, 1.0f);

    TextRendering_PrintString(window, " Projection matrix        Camera                    In NDC", -1.0f, 1.0f-17*pad, 1.0f);
    TextRendering_PrintMatrixVectorProductDivW(window, projection, p_camera, -1.0f, 1.0f-18*pad, 1.0f);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    glm::vec2 a = glm::vec2(-1, -1);
    glm::vec2 b = glm::vec2(+1, +1);
    glm::vec2 p = glm::vec2( 0,  0);
    glm::vec2 q = glm::vec2(width, height);

    glm::mat4 viewport_mapping = Matrix(
        (q.x - p.x)/(b.x-a.x), 0.0f, 0.0f, (b.x*p.x - a.x*q.x)/(b.x-a.x),
        0.0f, (q.y - p.y)/(b.y-a.y), 0.0f, (b.y*p.y - a.y*q.y)/(b.y-a.y),
        0.0f , 0.0f , 1.0f , 0.0f ,
        0.0f , 0.0f , 0.0f , 1.0f
    );

    TextRendering_PrintString(window, "                                                       |  ", -1.0f, 1.0f-22*pad, 1.0f);
    TextRendering_PrintString(window, "                            .--------------------------'  ", -1.0f, 1.0f-23*pad, 1.0f);
    TextRendering_PrintString(window, "                            V                           ", -1.0f, 1.0f-24*pad, 1.0f);

    TextRendering_PrintString(window, " Viewport matrix           NDC      In Pixel Coords.", -1.0f, 1.0f-25*pad, 1.0f);
    TextRendering_PrintMatrixVectorProductMoreDigits(window, viewport_mapping, p_ndc, -1.0f, 1.0f-26*pad, 1.0f);
}

// Escrevemos na tela o n�mero de balas que o jogador ainda possui
void TextRendering_ShowBullets(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float pad = TextRendering_LineHeight(window);

    char buffer[80];
    snprintf(buffer, 80, "Bullets = %d/6\n", num_shots);

    TextRendering_PrintString(window, buffer, -1.0f+pad/2, -0.9+pad/2, 1.5f);
}

// Escrevemos na tela o n�mero de vidas que o jogador ainda possui
void TextRendering_ShowLifes(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float pad = TextRendering_LineHeight(window);

    char buffer[80];
    snprintf(buffer, 80, "Lifes = %d\n", num_lifes);

    TextRendering_PrintString(window, buffer, -1.0f+pad/2, 0.9+pad/2, 1.5f);
}

// Escrevemos na tela o n�mero de pe�as do foguete que o jogador ainda possui
void TextRendering_ShowPieces(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float pad = TextRendering_LineHeight(window);

    char buffer[80];
    snprintf(buffer, 80, "Pieces = %d/5\n", num_pieces);

    TextRendering_PrintString(window, buffer, -1.0f+pad/2, -1.0+pad/2, 1.5f);
}

// Escrevemos na tela o n�mero de balas que o jogador ainda possui
void TextRendering_ShowGameOver(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float pad = TextRendering_LineHeight(window);

    char buffer[80];
    snprintf(buffer, 80, "GAME OVER\n");

    TextRendering_PrintString(window, buffer, -0.32f+pad, 0.2f+pad, 3.0f);
}

// Escrevemos na tela a mensagem de win
void TextRendering_ShowWin(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float pad = TextRendering_LineHeight(window);

    char buffer[80];
    snprintf(buffer, 80, "YOU'VE WON!!\n");

    TextRendering_PrintString(window, buffer, -0.35f+pad, pad, 3.0f);
}

// Escrevemos na tela o tempo restante at� que o oxig�nio acabe
void TextRendering_ShowTime(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float pad = TextRendering_LineHeight(window);

    int minutes = 9 - floor(glfwGetTime()/60.0f);
    int seconds = 59 - floor(fmod(glfwGetTime(), 60.0f));

    char buffer[80];
    if(seconds < 10){
        snprintf(buffer, 80, "0%d:0%d", minutes, seconds);
    }
    else{
        snprintf(buffer, 80, "0%d:%d", minutes, seconds);
    }


    TextRendering_PrintString(window, buffer, -0.1f+pad/3 , 0.9+pad/2, 2.0f);
}

void TextRendering_ShowPoints(GLFWwindow* window, int points)
{
    if ( !g_ShowInfoText )
        return;

    float pad = TextRendering_LineHeight(window);

    char buffer[80];
    snprintf(buffer, 80, "Points: %d", points);

    TextRendering_PrintString(window, buffer, 0.6f+pad/2 , -1.0+pad/2 , 1.5f);
}

void TextRendering_ShowBuyUpgrade(GLFWwindow* window, int points)
{
    if ( !g_ShowInfoText )
        return;

    float pad = TextRendering_LineHeight(window);

    char buffer[80];
    snprintf(buffer, 80, "Press [E] to get a random upgrade. (%d)", points);

    TextRendering_PrintString(window, buffer, -0.5f+pad , -0.7+pad/2 , 1.0f);
}

void TextRendering_ShowMessageExtraLife(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float pad = TextRendering_LineHeight(window);

    char buffer[80];
    snprintf(buffer, 80, "Extra Life");

    TextRendering_PrintString(window, buffer, -0.3f+pad, 0.2+pad, 2.0f);
}

void TextRendering_ShowMessageIncDamage(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float pad = TextRendering_LineHeight(window);

    char buffer[80];
    snprintf(buffer, 80, "Damage Increased");

    TextRendering_PrintString(window, buffer, -0.35f+pad, 0.2+pad, 2.0f);
}

void TextRendering_ShowMessageIncSpeed(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float pad = TextRendering_LineHeight(window);

    char buffer[80];
    snprintf(buffer, 80, "Speed Increased");

    TextRendering_PrintString(window, buffer, -0.35f+pad, 0.2+pad, 2.0f);
}

void TextRendering_ShowMessageInsufficientPoints(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float pad = TextRendering_LineHeight(window);

    char buffer[80];
    snprintf(buffer, 80, "Insufficient Points");

    TextRendering_PrintString(window, buffer, -0.4f+pad, 0.2+pad, 2.0f);
}



// Escrevemos na tela o n�mero de quadros renderizados por segundo (frames per
// second).
void TextRendering_ShowFramesPerSecond(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    // Vari�veis est�ticas (static) mant�m seus valores entre chamadas
    // subsequentes da fun��o!
    static float old_seconds = (float)glfwGetTime();
    static int   ellapsed_frames = 0;
    static char  buffer[20] = "?? fps";
    static int   numchars = 7;

    ellapsed_frames += 1;

    // Recuperamos o n�mero de segundos que passou desde a execu��o do programa
    float seconds = (float)glfwGetTime();

    // N�mero de segundos desde o �ltimo c�lculo do fps
    float ellapsed_seconds = seconds - old_seconds;

    if ( ellapsed_seconds > 1.0f )
    {
        numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);

        old_seconds = seconds;
        ellapsed_frames = 0;
    }

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, buffer, 1.0f-(numchars + 1)*charwidth, 1.0f-lineheight, 1.0f);
}

// set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
// vim: set spell spelllang=pt_br :

