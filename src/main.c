#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

// Defines
#define LARGURA_TELA 800
#define ALTURA_TELA 600

#define LARGURA_JOGADOR 40
#define ALTURA_JOGADOR 40
#define FORCA_PULO_JOGADOR 12.0f
#define VELOCIDADE_HORIZONTAL_JOGADOR 5.0f
#define GRAVIDADE 0.5f
#define VELOCIDADE_MAXIMA_QUEDA 10.0f
#define MARGEM_SEGURANCA_PULO 20

#define NUMERO_PLATAFORMAS 10
#define LARGURA_PLATAFORMA 100
#define ALTURA_PLATAFORMA 20
#define ESPACAMENTO_Y_MIN_PLATAFORMA 100
#define ESPACAMENTO_Y_MAX_PLATAFORMA 150
#define DESLOCAMENTO_X_MAX_PLATAFORMA 110

#define NUMERO_MOEDAS 20
#define TAMANHO_MOEDA 15
#define TAMANHO_CELULA_GRADE_MOEDA 20

//Estruturas
typedef struct {
    Rectangle ret;
    Vector2 velocidade;
    bool podePular;
    int pontuacao;
} Jogador;

typedef struct {
    Rectangle ret;
    bool ativa;
    Color cor;
} Plataforma;

typedef struct {
    Rectangle ret;
    bool ativa;
    Color cor;
} Moeda;

typedef enum {
    ESTADO_MENU,
    ESTADO_JOGANDO,
    ESTADO_FIM_DE_JOGO
} EstadoJogo;

// Variáveis Globais
Jogador jogador;
Plataforma plataformas[NUMERO_PLATAFORMAS];
Moeda moedas[NUMERO_MOEDAS];
float deslocamentoCameraY = 0;
float yPlataformaMaisAlta = 0;
float yInicialJogador = 0;
EstadoJogo estadoAtualJogo;
float alturaMaxAlcancadaExibida = 0;

void InicializarJogo(void);
void AtualizarJogo(void);
void DesenharJogo(void);
void InicializarJogador(void);
void InicializarPlataformas(void);
void InicializarMoedas(void);
void AtualizarJogador(void);
void AtualizarPlataformas(void);
void AtualizarMoedas(void);
void PosicionarNovaPlataforma(Plataforma* plataforma, float yTopoConhecido, float xAtualJogador);
void PosicionarNovaMoeda(Moeda* moeda, float xPlataforma, float yPlataforma);
float CalcularAlturaMaximaPulo(void);
int ObterEspacamentoYPlataformaAleatorio(float alturaCalculadaPuloMax);
void AtualizarMenu(void);
void DesenharMenu(void);
void AtualizarFimDeJogo(void);
void DesenharFimDeJogo(void);


int main(void) {
    InitWindow(LARGURA_TELA, ALTURA_TELA, "Sky Hopper - Raylib");
    InitAudioDevice();

    estadoAtualJogo = ESTADO_MENU;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        switch (estadoAtualJogo) {
        case ESTADO_MENU:
            AtualizarMenu();
            DesenharMenu();
            break;
        case ESTADO_JOGANDO:
            AtualizarJogo();
            DesenharJogo();
            break;
        case ESTADO_FIM_DE_JOGO:
            AtualizarFimDeJogo();
            DesenharFimDeJogo();
            break;
        }
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}

Rectangle retanguloBotaoIniciarMenu;
Rectangle retanguloBotaoVoltarMenuFimDeJogo;

void AtualizarMenu(void) {
    retanguloBotaoIniciarMenu = (Rectangle){ LARGURA_TELA / 2.0f - 120, ALTURA_TELA / 2.0f - 30, 240, 60 };

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(GetMousePosition(), retanguloBotaoIniciarMenu)) {
            estadoAtualJogo = ESTADO_JOGANDO;
            InicializarJogo();
        }
    }
}

void DesenharMenu(void) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    const char* textoTitulo = "Sky Hopper";
    int tamanhoFonteTitulo = 70;
    int larguraTextoTitulo = MeasureText(textoTitulo, tamanhoFonteTitulo);
    DrawText(textoTitulo, LARGURA_TELA / 2 - larguraTextoTitulo / 2, ALTURA_TELA / 4, tamanhoFonteTitulo, DARKGRAY);

    bool mouseSobreBotaoIniciar = CheckCollisionPointRec(GetMousePosition(), retanguloBotaoIniciarMenu);
    DrawRectangleRec(retanguloBotaoIniciarMenu, mouseSobreBotaoIniciar ? SKYBLUE : LIGHTGRAY);
    const char* textoBotaoIniciar = "Iniciar Jogo";
    int tamanhoFonteBotaoIniciar = 30;
    int larguraTextoBotaoIniciar = MeasureText(textoBotaoIniciar, tamanhoFonteBotaoIniciar);
    DrawText(textoBotaoIniciar, retanguloBotaoIniciarMenu.x + (retanguloBotaoIniciarMenu.width - larguraTextoBotaoIniciar) / 2, retanguloBotaoIniciarMenu.y + 15, tamanhoFonteBotaoIniciar, mouseSobreBotaoIniciar ? DARKBLUE : DARKGRAY);

    DrawText("Use as SETAS para mover, ESPACO para pular", LARGURA_TELA / 2 - MeasureText("Use as SETAS para mover, ESPACO para pular", 20) / 2, ALTURA_TELA - 50, 20, GRAY);

    EndDrawing();
}

void AtualizarFimDeJogo(void) {
    retanguloBotaoVoltarMenuFimDeJogo = (Rectangle){ LARGURA_TELA / 2.0f - 120, ALTURA_TELA - 150, 240, 50 };

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(GetMousePosition(), retanguloBotaoVoltarMenuFimDeJogo)) {
            estadoAtualJogo = ESTADO_MENU;
        }
    }
}

void DesenharFimDeJogo(void) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    const char* textoFimDeJogo = "GAME OVER";
    int tamanhoFonteFimDeJogo = 80;
    int larguraTextoFimDeJogo = MeasureText(textoFimDeJogo, tamanhoFonteFimDeJogo);
    DrawText(textoFimDeJogo, LARGURA_TELA / 2 - larguraTextoFimDeJogo / 2, ALTURA_TELA / 4, tamanhoFonteFimDeJogo, RED);

    char bufferTextoPontuacao[32];
    sprintf(bufferTextoPontuacao, "Score Final: %d", jogador.pontuacao);
    int tamanhoFontePontuacao = 30;
    int larguraTextoPontuacao = MeasureText(bufferTextoPontuacao, tamanhoFontePontuacao);
    DrawText(bufferTextoPontuacao, LARGURA_TELA / 2 - larguraTextoPontuacao / 2, ALTURA_TELA / 2 - 30, tamanhoFontePontuacao, DARKGRAY);

    char bufferTextoAltura[32];
    sprintf(bufferTextoAltura, "Altura Maxima: %.0f", alturaMaxAlcancadaExibida);
    int tamanhoFonteAltura = 30;
    int larguraTextoAltura = MeasureText(bufferTextoAltura, tamanhoFonteAltura);
    DrawText(bufferTextoAltura, LARGURA_TELA / 2 - larguraTextoAltura / 2, ALTURA_TELA / 2 + 10, tamanhoFonteAltura, DARKGRAY);

    bool mouseSobreBotaoVoltar = CheckCollisionPointRec(GetMousePosition(), retanguloBotaoVoltarMenuFimDeJogo);
    DrawRectangleRec(retanguloBotaoVoltarMenuFimDeJogo, mouseSobreBotaoVoltar ? SKYBLUE : LIGHTGRAY);
    const char* textoBotaoVoltar = "Voltar ao Menu";
    int tamanhoFonteBotaoVoltar = 20;
    int larguraTextoBotaoVoltar = MeasureText(textoBotaoVoltar, tamanhoFonteBotaoVoltar);
    DrawText(textoBotaoVoltar, retanguloBotaoVoltarMenuFimDeJogo.x + (retanguloBotaoVoltarMenuFimDeJogo.width - larguraTextoBotaoVoltar) / 2, retanguloBotaoVoltarMenuFimDeJogo.y + 15, tamanhoFonteBotaoVoltar, mouseSobreBotaoVoltar ? DARKBLUE : DARKGRAY);

    EndDrawing();
}

float CalcularAlturaMaximaPulo(void) {
    float v0 = FORCA_PULO_JOGADOR;
    float g = GRAVIDADE;
    if (g <= 0) return ALTURA_TELA * 10;
    return (v0 * v0) / (2.0f * g) + (v0 / 2.0f);
}

int ObterEspacamentoYPlataformaAleatorio(float alturaCalculadaPuloMax) {
    float espacamentoConfortavelMax = alturaCalculadaPuloMax - MARGEM_SEGURANCA_PULO;
    if (espacamentoConfortavelMax < MARGEM_SEGURANCA_PULO) espacamentoConfortavelMax = MARGEM_SEGURANCA_PULO;
    if (espacamentoConfortavelMax < 10.0f) espacamentoConfortavelMax = 10.0f;

    float espacamentoRealMax = ESPACAMENTO_Y_MAX_PLATAFORMA;
    if (espacamentoRealMax > espacamentoConfortavelMax) espacamentoRealMax = espacamentoConfortavelMax;
    if (espacamentoRealMax < 10.0f) espacamentoRealMax = 10.0f;

    float espacamentoRealMin = ESPACAMENTO_Y_MIN_PLATAFORMA;
    if (espacamentoRealMin < 10.0f) espacamentoRealMin = 10.0f;

    if (espacamentoRealMin > espacamentoRealMax) espacamentoRealMin = espacamentoRealMax;

    int range = (int)(espacamentoRealMax - espacamentoRealMin);
    if (range < 0) range = 0;
    int espacamento = (int)espacamentoRealMin + (rand() % (range + 1));
    return espacamento;
}

void InicializarJogo(void) {
    srand(time(NULL));
    InicializarJogador();
    InicializarPlataformas();
    InicializarMoedas();
    deslocamentoCameraY = 0;
    alturaMaxAlcancadaExibida = 0;
}

void InicializarJogador(void) {
    jogador.ret = (Rectangle){ LARGURA_TELA / 2.0f - LARGURA_JOGADOR / 2.0f, ALTURA_TELA / 2.0f, LARGURA_JOGADOR, ALTURA_JOGADOR };
    jogador.velocidade = (Vector2){ 0, 0 };
    jogador.podePular = false;
    jogador.pontuacao = 0;
}

void InicializarPlataformas(void) {
    plataformas[0].ret.width = LARGURA_PLATAFORMA;
    plataformas[0].ret.height = ALTURA_PLATAFORMA;
    plataformas[0].ret.x = LARGURA_TELA / 2.0f - LARGURA_PLATAFORMA / 2.0f;
    plataformas[0].ret.y = ALTURA_TELA - ALTURA_PLATAFORMA * 3.0f;
    plataformas[0].ativa = true;
    plataformas[0].cor = DARKGREEN;

    jogador.ret.x = plataformas[0].ret.x + (LARGURA_PLATAFORMA / 2.0f) - (LARGURA_JOGADOR / 2.0f);
    jogador.ret.y = plataformas[0].ret.y - ALTURA_JOGADOR;
    jogador.velocidade.y = 0;
    jogador.podePular = true;

    yInicialJogador = jogador.ret.y;

    yPlataformaMaisAlta = plataformas[0].ret.y;
    float alturaMaxPulo = CalcularAlturaMaximaPulo();
    float yAtual = plataformas[0].ret.y;
    float centroXPlataformaAnterior = plataformas[0].ret.x + LARGURA_PLATAFORMA / 2.0f;

    for (int i = 1; i < NUMERO_PLATAFORMAS; i++) {
        int espacamentoVertical = ObterEspacamentoYPlataformaAleatorio(alturaMaxPulo);
        yAtual -= espacamentoVertical;
        int direcaoDeslocamentoX = (rand() % 2 == 0) ? 1 : -1;
        int deslocamentoXAleatorio = rand() % (DESLOCAMENTO_X_MAX_PLATAFORMA + 1);
        float novoCentroXPlataforma = centroXPlataformaAnterior + (direcaoDeslocamentoX * deslocamentoXAleatorio);
        float novoXPlataforma = novoCentroXPlataforma - LARGURA_PLATAFORMA / 2.0f;

        if (novoXPlataforma < 0) novoXPlataforma = 0;
        if (novoXPlataforma + LARGURA_PLATAFORMA > LARGURA_TELA) novoXPlataforma = LARGURA_TELA - LARGURA_PLATAFORMA;

        plataformas[i].ret.x = novoXPlataforma;
        plataformas[i].ret.y = yAtual;
        plataformas[i].ret.width = LARGURA_PLATAFORMA;
        plataformas[i].ret.height = ALTURA_PLATAFORMA;
        plataformas[i].ativa = true;
        plataformas[i].cor = GREEN;
        centroXPlataformaAnterior = novoXPlataforma + LARGURA_PLATAFORMA / 2.0f;
        if (yAtual < yPlataformaMaisAlta) yPlataformaMaisAlta = yAtual;
    }
}

void PosicionarNovaPlataforma(Plataforma* plataforma, float yTopoConhecido, float xAtualJogador) {
    float alturaMaxPulo = CalcularAlturaMaximaPulo();
    int espacamentoVertical = ObterEspacamentoYPlataformaAleatorio(alturaMaxPulo);
    float centroJogadorX = xAtualJogador + LARGURA_JOGADOR / 2.0f;
    int direcaoDeslocamentoX = (rand() % 2 == 0) ? 1 : -1;
    int deslocamentoXAleatorio = rand() % (DESLOCAMENTO_X_MAX_PLATAFORMA + 1);
    float novoCentroXPlataforma = centroJogadorX + (direcaoDeslocamentoX * deslocamentoXAleatorio);
    float novoXPlataforma = novoCentroXPlataforma - LARGURA_PLATAFORMA / 2.0f;

    if (novoXPlataforma < 0) novoXPlataforma = 0;
    if (novoXPlataforma + LARGURA_PLATAFORMA > LARGURA_TELA) novoXPlataforma = LARGURA_TELA - LARGURA_PLATAFORMA;

    plataforma->ret.x = novoXPlataforma;
    plataforma->ret.y = yTopoConhecido - espacamentoVertical;
    plataforma->ativa = true;
    plataforma->cor = GREEN;
}

void InicializarMoedas(void) {
    for (int i = 0; i < NUMERO_MOEDAS; i++) moedas[i].ativa = false;
    int indiceMoeda = 0;
    for (int i = 0; i < NUMERO_PLATAFORMAS && indiceMoeda < NUMERO_MOEDAS; i++) {
        if (plataformas[i].ativa && (rand() % 2 == 0)) {
            PosicionarNovaMoeda(&moedas[indiceMoeda], plataformas[i].ret.x, plataformas[i].ret.y);
            if (moedas[indiceMoeda].ativa) indiceMoeda++;
        }
    }
}

void PosicionarNovaMoeda(Moeda* moeda, float xPlataforma, float yPlataforma) {
    if (rand() % 3 == 0) {
        moeda->ret.width = TAMANHO_MOEDA;
        moeda->ret.height = TAMANHO_MOEDA;
        float centroPlataformaX = xPlataforma + LARGURA_PLATAFORMA / 2;
        moeda->ret.x = ((int)(centroPlataformaX / TAMANHO_CELULA_GRADE_MOEDA) * TAMANHO_CELULA_GRADE_MOEDA) - TAMANHO_MOEDA / 2;
        moeda->ret.y = yPlataforma - TAMANHO_MOEDA - (rand() % (TAMANHO_CELULA_GRADE_MOEDA * 2));
        moeda->cor = GOLD;
        moeda->ativa = true;
    }
    else {
        moeda->ativa = false;
    }
}

void AtualizarJogo(void) {
    AtualizarJogador();
    AtualizarPlataformas();
    AtualizarMoedas();

    if (jogador.ret.y - deslocamentoCameraY > ALTURA_TELA + ALTURA_JOGADOR) {
        estadoAtualJogo = ESTADO_FIM_DE_JOGO;
    }
}

void AtualizarJogador(void) {
    if (IsKeyDown(KEY_LEFT)) jogador.ret.x -= VELOCIDADE_HORIZONTAL_JOGADOR;
    if (IsKeyDown(KEY_RIGHT)) jogador.ret.x += VELOCIDADE_HORIZONTAL_JOGADOR;

    if (jogador.ret.x < 0) jogador.ret.x = 0;
    if (jogador.ret.x + jogador.ret.width > LARGURA_TELA) jogador.ret.x = LARGURA_TELA - jogador.ret.width;

    if (jogador.podePular && IsKeyPressed(KEY_SPACE)) {
        jogador.velocidade.y = -FORCA_PULO_JOGADOR;
        jogador.podePular = false;
    }

    jogador.velocidade.y += GRAVIDADE;
    if (jogador.velocidade.y > VELOCIDADE_MAXIMA_QUEDA) jogador.velocidade.y = VELOCIDADE_MAXIMA_QUEDA;
    jogador.ret.y += jogador.velocidade.y;

    float alturaAtualExibida = yInicialJogador - jogador.ret.y;
    if (alturaAtualExibida > alturaMaxAlcancadaExibida) {
        alturaMaxAlcancadaExibida = alturaAtualExibida;
    }

    float jogadorNaTelaY = jogador.ret.y - deslocamentoCameraY;
    if (jogador.velocidade.y < 0 && jogadorNaTelaY < ALTURA_TELA * 0.4f) {
        deslocamentoCameraY += (jogadorNaTelaY - ALTURA_TELA * 0.4f);
    }

    jogador.podePular = false;
    for (int i = 0; i < NUMERO_PLATAFORMAS; i++) {
        if (plataformas[i].ativa && jogador.velocidade.y >= 0) {
            if ((jogador.ret.x < plataformas[i].ret.x + plataformas[i].ret.width &&
                jogador.ret.x + jogador.ret.width > plataformas[i].ret.x) &&
                (jogador.ret.y + jogador.ret.height + jogador.velocidade.y >= plataformas[i].ret.y &&
                    jogador.ret.y + jogador.ret.height <= plataformas[i].ret.y + ALTURA_PLATAFORMA / 1.5f))
            {
                jogador.ret.y = plataformas[i].ret.y - jogador.ret.height;
                jogador.velocidade.y = 0;
                jogador.podePular = true;
                break;
            }
        }
    }
}

void AtualizarPlataformas(void) {
    for (int i = 0; i < NUMERO_PLATAFORMAS; i++) {
        if (plataformas[i].ativa && (plataformas[i].ret.y - deslocamentoCameraY > ALTURA_TELA)) {
            PosicionarNovaPlataforma(&plataformas[i], yPlataformaMaisAlta, jogador.ret.x);
            yPlataformaMaisAlta = plataformas[i].ret.y;

            for (int j = 0; j < NUMERO_MOEDAS; j++) {
                if (!moedas[j].ativa) {
                    PosicionarNovaMoeda(&moedas[j], plataformas[i].ret.x, plataformas[i].ret.y);
                    break;
                }
            }
        }
    }

    bool encontrouPlataformaAtiva = false;
    float novoYMinimoGeral = 0;
    for (int idxPlat = 0; idxPlat < NUMERO_PLATAFORMAS; idxPlat++) {
        if (plataformas[idxPlat].ativa) {
            if (!encontrouPlataformaAtiva) {
                novoYMinimoGeral = plataformas[idxPlat].ret.y;
                encontrouPlataformaAtiva = true;
            }
            else {
                if (plataformas[idxPlat].ret.y < novoYMinimoGeral) {
                    novoYMinimoGeral = plataformas[idxPlat].ret.y;
                }
            }
        }
    }

    if (encontrouPlataformaAtiva) {
        yPlataformaMaisAlta = novoYMinimoGeral;
    }
    else {
        int espacamento = ObterEspacamentoYPlataformaAleatorio(CalcularAlturaMaximaPulo());
        yPlataformaMaisAlta = deslocamentoCameraY - espacamento - ALTURA_JOGADOR;
    }
}

void AtualizarMoedas(void) {
    for (int i = 0; i < NUMERO_MOEDAS; i++) {
        if (moedas[i].ativa) {
            if (CheckCollisionRecs(jogador.ret, moedas[i].ret)) {
                moedas[i].ativa = false;
                jogador.pontuacao += 10;
            }
            else if (moedas[i].ret.y - deslocamentoCameraY > ALTURA_TELA) {
                moedas[i].ativa = false;
            }
        }
    }
}

void DesenharJogo(void) {
    BeginDrawing();
    ClearBackground(LIGHTGRAY);

    for (int i = 0; i < NUMERO_PLATAFORMAS; i++) {
        if (plataformas[i].ativa) {
            DrawRectangleRec((Rectangle) { plataformas[i].ret.x, plataformas[i].ret.y - deslocamentoCameraY, plataformas[i].ret.width, plataformas[i].ret.height }, plataformas[i].cor);
        }
    }

    for (int i = 0; i < NUMERO_MOEDAS; i++) {
        if (moedas[i].ativa) {
            DrawRectangleRec((Rectangle) { moedas[i].ret.x, moedas[i].ret.y - deslocamentoCameraY, moedas[i].ret.width, moedas[i].ret.height }, moedas[i].cor);
        }
    }

    DrawRectangleRec((Rectangle) { jogador.ret.x, jogador.ret.y - deslocamentoCameraY, jogador.ret.width, jogador.ret.height }, BLUE);

    DrawText(TextFormat("Score: %d", jogador.pontuacao), 10, 10, 20, DARKGRAY);
    float alturaExibida = yInicialJogador - jogador.ret.y;
    DrawText(TextFormat("Altura: %.0f", alturaExibida), 10, 30, 20, DARKGRAY);

    EndDrawing();
}