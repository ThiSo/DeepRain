# DeepRain

### Trabalho Final da cadeira de Fundamentos de Computação Gráfica - semestre 2023/1

#### Integrantes da dupla: Thiago Silva Oliveski (323982) e Vitor Pedrollo dos Santos (312948)

### Descrição da aplicação
A aplicação gráfica desenvolvida em C++ e OpenGL para o trabalho final consiste em um jogo FPS/Tower Defense inspirado pelos jogos Risk of Rain 2 e Deep Rock Galactic.

O jogo gira em torno da sobrevivência de um astronauta que estava viajando pelo espaço em uma nave e, por conta de uma pane, acabou caindo em um planeta desconhecido.
O objetivo principal do astronauta é encontrar pelo planeta pedaços da nave que foram perdidos na queda para conseguir reconstruí-la. No entanto, a tarefa não será tão fácil pois esse planeta é povoado por alienígenas hostis. Então, além de encontrar todas as partes que faltam da nave, o astronauta deve eliminar alienigenas e acumular pontos que poderão ser trocados em melhorias no seu traje ou na sua arma. Depois de juntar todas as peças que faltam da nave, o astronauta deve proteger a nave de todos os perigos para que ele possa fugir de lá.

### Contribuições dos membros da dupla
Thiago:
        Modelos 3D e suas respectivas texturas (skybox, monstros, plano, entre outros);
        Sistemas de movimentação e câmera do jogador e dos monstros;
        Sistema de tiros;
        Sistema de pulo;
        Iluminação difusa e de blinn-phong;
        Estruturação de objetos;
        Gourard Shading;
        Curvas de Bézier;
        Testes de colisão e hitboxes para alguns objetos (monstros, monte, bordas do mapa);
        Bug fixes.

Vitor:
        Modelos 3D e suas respectivas texturas (boss, arma, árvores, entre outros);
        Sistemas de movimentação e câmera do boss;
        Sistema de upgrades para os status do jogador;
        Sistema de pulo;
        Estruturação de objetos;
        Respawn de monstros com base no tempo decorrido;
        Cutscenes de vitória e derrota;
        Testes de colisão e hitboxes para alguns objetos (boss, monte, bordas do mapa);
        Bug fixes.

### Uso de IA's no trabalho
O chatGPT foi utilizado para auxiliar na lógica de dois dos quatro testes de colisão utilizados no jogo (colisão ponto-esfera e colisão ponto-plano, ambas documentadas no código com o prompt do chat). Para esta aplicação em específico ele se provou bem útil, gerando exemplos de código 
que podiam facilmente ser adaptados para funcionar da forma que queríamos. 
Também tentamos utilizá-lo para resolver um problema que estavamos tendo com a arma não acompanhando corretamente a movimentação da câmera, 
porém o auxílio para este caso não foi satisfatório, e só foi possível resolver o problema conversando com o professor na apresentação parcial.

### Uso dos conceitos e requisitos na aplicação
Utilização das Matrizes vistas em aula: Todas as matrizes utilizadas no programa foram reutilizadas e adaptadas a partir dos códigos dos laboratórios;

Objetos representados através de malhas poligonais complexas: Foram utilizados 15 objetos diferentes, carregados no programa a partir de arquivos .OBJ.

Alguns exemplos de objetos seguem abaixo: <br>

![image](https://github.com/ThiSo/DeepRain/assets/81988524/9aeac2bb-4de9-4d28-bbd1-813f418de96b)
<br>imagem 1 - modelo da nave espacial utilizado na cutscene de vitória

![image](https://github.com/ThiSo/DeepRain/assets/81988524/0f4704bf-9a0e-41b8-8f35-ad361db8e5a9)
<br>imagem 2 - modelo do boss 

![image](https://github.com/ThiSo/DeepRain/assets/81988524/c0e785a1-8d2b-4c6c-9af4-51a101ff4bdf)
<br>imagem 3 - modelo do coelho, com iluminação de Blinn-Phong

![image](https://github.com/ThiSo/DeepRain/assets/81988524/8bb5cbfc-3cbc-471e-8db5-4e0360043550)
<br>imagem 4 - modelos das árvores, com Gourard Shading

Transformações geométricas de objetos virtuais: Este conceito é aplicado no sistema de tiro do jogo. Quando o usuário clica com o botão 
esquerdo do mouse, uma nova instância do objeto Projectile é criada e passa por transformações geométricas distintas ao longo do seu tempo de vida;

Controle de câmeras virtuais: No jogo, o usuário controla uma free camera na maior parte do tempo; além disso, são utilizadas câmeras look-at
para as cutscenes presentes (cutscene de spawn do boss, cutscene de vitória e de derrota);

Cópia de instâncias com matrizes de modelagem distintas: Objetos comuns no cenário (como pedras e árvores) compartilham o mesmo conjunto de vértices
e possuem translações diferentes para seu desenho na tela;

Testes de intersecção entre objetos virtuais: No jogo, utilizamos quatro testes de intersecção. São eles <br> 
Teste ponto-plano, que checa a colisão do jogador com o plano e não o permite "cair do chão"; <br>
Teste ponto-esfera que serve para checar a colisão entre o jogador e os monstros, assim como para controle da cutscene de spawn do boss; <br>
Teste esfera-esfera que serve para checar a colisão entre o boss e a nave (o que resulta num game over); <br>
Teste ponto-cubo que checa a colisão do jogador com as bordas do mapa e com o monte presente no cenário.

Modelos de iluminação de objetos geométricos: Quanto aos modelos de iluminação, quase todos os objetos do jogo utilizam um modelo de iluminação
difusa (como exemplo, ver imagem 2 acima). Apenas o modelo do coelho (imagem 3) apresenta modelo de iluminação de Blinn-Phong; <br>
Quanto ao modelo de interpolação, maioria dos objetos utilizam o modelo de Phong (imagens 1, 2 e 3) e apenas o modelo das árvores utiliza
o modelo de Gourard (imagem 4).

Mapeamento de texturas: Todos os 15 objetos possuem as cores definidas por texturas representadas por imagens. O mapeamento destas texturas é feito
com base no código dos laboratórios. Vale comentar que na textura do plano, forçamos as coordenadas de textura a sairem do intervalo [0,1] para poder
utilizar o método GL_MIRRORED_REPEAT e não ocorrer perda de qualidade.

Curvas de Bézier: Utilizamos duas curvas de bézier cúbicas para movimentação do objeto fly_monster (imagem 5 abaixo), que faz um ciclo de vôo 
ao redor do monte.

![image](https://github.com/ThiSo/DeepRain/assets/81988524/fc6de7fd-19b3-4187-bcc2-1829a74b766e)
<br>imagem 5 - monstro com movimento definido por duas curvas de bézier cúbicas

Animações de movimento baseadas no tempo: Todas as movimentações de objetos do programa (jogador, monstros, boss, etc...) se baseiam no tempo, utilizando um parâmetro delta_t, para que não hajam mudanças dependentes da CPU em que o jogo está sendo executado.

### Manual de utilização 
Tecla W - move o jogador para frente;<br>
Tecla A - move o jogador para a esquerda;<br>
Tecla S - move o jogador para trás;<br>
Tecla D - move o jogador para direita;<br>
Tecla R - recarrega as balas da arma;<br>
Tecla E - pode ser utilizada quando próximo do monte para escalá-lo ou quando próximo de uma cápsula para compra de upgrades;<br>
Tecla espaço - realiza um pulo;<br>
Botão esquerdo do mouse - atira com a arma;<br>

### Tutorial para compilação e execução
Para compilar e executar o código, basta abrir o arquivo DeepRain.cbp (presente na pasta DeepRain) com a IDE Code::Blocks e clicar na opção
"build and run" no topo da interface (ou pressionar a tecla F9, alternativamente).
Alternativamente, pode-se encontrar o arquivo binário pré-compilado em DeepRain/bin/Debug/main.exe

### Link para showcase do jogo no youtube
https://www.youtube.com/watch?v=WOX067mlLYQ




