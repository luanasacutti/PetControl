// painel_api_poppins.c
// Painel (Raylib) integrado com API usando curl.exe
// Estilo: Verde Soft, Fonte Poppins Regular/SemiBold, UI moderna e legível
// Observação: carrega fontes tentando tanto "../assets/fonts/..." quanto "assets/fonts/...".
// Compile: gcc painel_api_poppins.c -o PetControl.exe -lraylib -static -mwindows
// (ajuste flags conforme seu ambiente)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "raylib.h"

#define MAX_REG 1024

// Layout constants
#define WINDOW_W 1100
#define WINDOW_H 700
#define TOP_BAR_H 70
#define SIDE_MENU_W 220
#define ROW_HEIGHT 48
#define TABLE_X 240
#define TABLE_W 840
#define TABLE_TOP 265
#define TABLE_VISIBLE_H 360
#define SCROLLBAR_W 10

// Path to curl executable (ajuste se necessário)
#define CURL_EXE "C:\\\\curl\\\\curl.exe"

// ------------------------------------------------------------
// Estrutura Cliente
// ------------------------------------------------------------
typedef struct {
    int id;
    char nome[128];
    char email[128];
    char telefone[64];
    char cpf_cnpj[64];
    char plano[128];
    char vencimento[16];
} Cliente;

static Cliente clientes[MAX_REG];
static int totalClientes = 0;

// Scroll state
static float scrollY = 0.0f;
static float scrollSpeed = 32.0f;

// Fonts
static Font gFontRegular = { 0 };
static Font gFontSemiBold = { 0 };

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------
static float clampf(float v, float a, float b) {
    if (v < a) return a;
    if (v > b) return b;
    return v;
}

static void safe_strcpy(char *dst, const char *src, size_t dstsz) {
    if (!dst || dstsz == 0) return;
    if (!src) { dst[0] = '\0'; return; }
    strncpy(dst, src, dstsz - 1);
    dst[dstsz - 1] = '\0';
}

static Color ColorLerp(Color a, Color b, float t) {
    if (t < 0) t = 0;
    if (t > 1) t = 1;
    return (Color){
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        (unsigned char)(a.a + (b.a - a.a) * t)
    };
}

static Color FadeAlpha(Color c, float f) {
    Color out = c;
    out.a = (unsigned char)(c.a * f);
    return out;
}

// ------------------------------------------------------------
// Calcular dias restantes
// ------------------------------------------------------------
int diasRestantes(const char *dataVenc) {
    if (!dataVenc || strlen(dataVenc) < 10) return 99999;
    int ano, mes, dia;

    if (sscanf(dataVenc, "%d-%d-%d", &ano, &mes, &dia) != 3)
        return 99999;

    struct tm tmv = {0};
    tmv.tm_year = ano - 1900;
    tmv.tm_mon = mes - 1;
    tmv.tm_mday = dia;
    tmv.tm_hour = 12;

    time_t t_venc = mktime(&tmv);
    if (t_venc == (time_t)-1) return 99999;

    time_t now = time(NULL);
    return (int)((t_venc - now) / 86400.0);
}

// ------------------------------------------------------------
// CSV escape
// ------------------------------------------------------------
static void csv_escape_and_write(FILE *f, const char *s) {
    if (!f) return;
    if (!s) { fputs("\"\"", f); return; }

    fputc('"', f);
    for (const char *p = s; *p; p++) {
        if (*p == '"') fputc('"', f);
        fputc(*p, f);
    }
    fputc('"', f);
}
// ------------------------------------------------------------
// Validação simples de e-mail
// ------------------------------------------------------------
static int email_valido(const char *e) {
    if (!e) return 0;
    const char *at = strchr(e, '@');
    if (!at) return 0;
    const char *dot = strchr(at, '.');
    if (!dot) return 0;
    return 1;
}

// ------------------------------------------------------------
// LOG: verificar se já foi enviado
// ------------------------------------------------------------
int emailJaEnviado(int id) {
    FILE *f = fopen("email_enviados.log", "r");
    if (!f) return 0;

    int idl;
    char data[64];
    while (fscanf(f, "%d;%63s", &idl, data) == 2) {
        if (idl == id) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

// ------------------------------------------------------------
// Registrar envio
// ------------------------------------------------------------
void registrarEnvioEmail(int id) {
    FILE *f = fopen("email_enviados.log", "a");
    if (!f) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(f, "%d;%04d-%02d-%02d\n",
            id,
            t->tm_year + 1900,
            t->tm_mon + 1,
            t->tm_mday);

    fclose(f);
}

// ------------------------------------------------------------
// Executar curl capturando resposta
// ------------------------------------------------------------
char *run_curl_capture(const char *cmd) {
#ifdef _WIN32
    FILE *fp = _popen(cmd, "r");
#else
    FILE *fp = popen(cmd, "r");
#endif
    if (!fp) return NULL;

    size_t cap = 8192;
    char *buf = malloc(cap);
    if (!buf) { 
#ifdef _WIN32
        _pclose(fp);
#else
        pclose(fp);
#endif
        return NULL;
    }

    size_t len = 0;
    while (!feof(fp)) {
        if (len + 1024 >= cap) {
            cap *= 2;
            char *tmp = realloc(buf, cap);
            if (!tmp) { free(buf); 
#ifdef _WIN32
                _pclose(fp);
#else
                pclose(fp);
#endif
                return NULL; 
            }
            buf = tmp;
        }
        size_t r = fread(buf + len, 1, 1024, fp);
        len += r;
    }

    if (len == 0) {
        buf[0] = '\0';
    } else {
        if (len >= cap) { buf[cap-1] = '\0'; }
        else buf[len] = '\0';
    }

#ifdef _WIN32
    _pclose(fp);
#else
    pclose(fp);
#endif
    return buf;
}

// ------------------------------------------------------------
// HTTP GET /api/clientes
// ------------------------------------------------------------
char *http_get_clients_json(void) {
    char cmd[1024];

    snprintf(cmd, sizeof(cmd),
             "%s -s \"http://localhost:3000/api/clientes\"",
             CURL_EXE);

    return run_curl_capture(cmd);
}


// ------------------------------------------------------------
// Parse do JSON simples e rápido
// ------------------------------------------------------------
int carregarClientesAPI(void) {
    char *json = http_get_clients_json();
    if (!json) return 0;

    totalClientes = 0;
    char *p = json;

    while ((p = strstr(p, "{")) != NULL) {
        char *end = strstr(p, "}");
        if (!end) break;

        size_t len = end - p + 1;
        char *obj = malloc(len + 1);
        if (!obj) break;
        strncpy(obj, p, len);
        obj[len] = '\0';

        Cliente *c = &clientes[totalClientes];
        memset(c, 0, sizeof(Cliente));

        char *k;

        k = strstr(obj, "\"id\":");
        if (k) sscanf(k, "\"id\":%d", &c->id);

        k = strstr(obj, "\"nome\":");
        if (k) sscanf(k, "\"nome\":\"%127[^\"]\"", c->nome);

        k = strstr(obj, "\"email\":");
        if (k) sscanf(k, "\"email\":\"%127[^\"]\"", c->email);

        k = strstr(obj, "\"telefone\":");
        if (k) sscanf(k, "\"telefone\":\"%63[^\"]\"", c->telefone);

        k = strstr(obj, "\"cpf_cnpj\":");
        if (k) sscanf(k, "\"cpf_cnpj\":\"%63[^\"]\"", c->cpf_cnpj);

        k = strstr(obj, "\"plano\":");
        if (k) sscanf(k, "\"plano\":\"%127[^\"]\"", c->plano);

        k = strstr(obj, "\"vencimento\":");
        if (k) sscanf(k, "\"vencimento\":\"%15[^\"]\"", c->vencimento);

        free(obj);

        totalClientes++;
        if (totalClientes >= MAX_REG) break;

        p = end + 1;
    }

    free(json);
    return totalClientes;
}

// Escape seguro para JSON
void json_escape(char *dst, const char *src, size_t size) {
    size_t j = 0;
    for (size_t i = 0; src[i] && j + 6 < size; i++) {
        char c = src[i];

        if (c == '\"' || c == '\\') {
            dst[j++] = '\\';
            dst[j++] = c;
        }
        else if ((unsigned char)c < 32) {
            j += snprintf(dst + j, size - j, "\\u%04x", c);
        }
        else {
            dst[j++] = c;
        }
    }
    dst[j] = '\0';
}

// ------------------------------------------------------------
// Enviar e-mail via API (POST)
// ------------------------------------------------------------
void enviarEmailAPI(Cliente *c) {
    printf(">>> enviarEmailAPI() chamado para: %s (%s)\n", c->nome, c->email);
fflush(stdout);

    if (!c) return;
    if (!email_valido(c->email)) return;
    if (emailJaEnviado(c->id)) return;

    char emailEsc[256], nomeEsc[256], vencEsc[64];

    json_escape(emailEsc, c->email, sizeof(emailEsc));
    json_escape(nomeEsc,  c->nome,  sizeof(nomeEsc));
    json_escape(vencEsc,  c->vencimento, sizeof(vencEsc));

    char payload[512];
    snprintf(payload, sizeof(payload),
        "{\"email\":\"%s\",\"nome\":\"%s\",\"vencimento\":\"%s\"}",
        emailEsc, nomeEsc, vencEsc
    );

    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
         "%s -s -X POST -H \"Content-Type: application/json\" "
         "-d \"%s\" \"http://localhost:3000/api/enviar-email\"",
         CURL_EXE, payload);

printf("\n=== CURL CMD ===\n%s\n=================\n", cmd);




    system(cmd);
    registrarEnvioEmail(c->id);
}

// ------------------------------------------------------------
// Exportar CSV
// ------------------------------------------------------------
void exportarRelatorioCSV(const char *fname) {
    FILE *f = fopen(fname, "w");
    if (!f) return;

    unsigned char bom[] = {0xEF,0xBB,0xBF};
    fwrite(bom, 1, 3, f);

    fprintf(f, "ID,Nome,Email,Telefone,CPF_CNPJ,Plano,Vencimento,Situacao,Dias\n");

    for (int i = 0; i < totalClientes; i++) {
        Cliente *c = &clientes[i];
        int dias = diasRestantes(c->vencimento);

        const char *sit =
            dias < 0 ? "Expirado" :
            dias <= 3 ? "A Vencer" :
                        "Ativo";

        fprintf(f, "%d,", c->id);
        csv_escape_and_write(f, c->nome); fprintf(f, ",");
        csv_escape_and_write(f, c->email); fprintf(f, ",");
        csv_escape_and_write(f, c->telefone); fprintf(f, ",");
        csv_escape_and_write(f, c->cpf_cnpj); fprintf(f, ",");
        csv_escape_and_write(f, c->plano); fprintf(f, ",");
        csv_escape_and_write(f, c->vencimento); fprintf(f, ",");
        csv_escape_and_write(f, sit);
        fprintf(f, ",%d\n", dias);
    }

    fclose(f);
}

// ------------------------------------------------------------
// Botão estilizado (Verde Soft) - usa gFontRegular (Poppins Regular)
// ------------------------------------------------------------
bool Botao(Rectangle r, const char *label) {
    Vector2 m = GetMousePosition();
    bool hover = CheckCollisionPointRec(m, r);
    bool released = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

    Color base       = (Color){245, 250, 247, 255};
    Color baseHover  = (Color){235, 246, 240, 255};
    Color border     = (Color){200, 215, 205, 180};
    Color shadow     = (Color){0, 0, 0, 28};
    Color textColor  = (Color){40, 60, 50, 255};

    DrawRectangleRounded((Rectangle){r.x+2, r.y+4, r.width, r.height}, 0.16f, 6, shadow);
    DrawRectangleRounded(r, 0.16f, 6, hover ? baseHover : base);
    DrawRectangleRoundedLines(r, 0.16f, 6, 1.0f, border);

    int fontSize = 18;
    Vector2 ts = MeasureTextEx(gFontRegular, label, fontSize, 0);

    DrawTextEx(
        gFontRegular, label,
        (Vector2){ r.x + (r.width - ts.x) * 0.5f, r.y + (r.height - ts.y) * 0.5f },
        fontSize, 0,
        textColor
    );

    return hover && released;
}

// ------------------------------------------------------------
// Card Bonito (Verde Soft) - título usa SemiBold
// ------------------------------------------------------------
void Card(Rectangle r, const char *titulo, int valor, Color cor) {
    Color shadow = (Color){0,0,0,36};
    Color topBright = ColorLerp(cor, (Color){255,255,255,255}, 0.12f);

    DrawRectangleRounded((Rectangle){r.x+4, r.y+6, r.width, r.height}, 0.22f, 12, shadow);
    DrawRectangleRounded(r, 0.22f, 12, cor);

    DrawRectangleRounded(
        (Rectangle){r.x, r.y, r.width, r.height*0.32f},
        0.22f, 12,
        FadeAlpha(topBright, 0.28f)
    );

    DrawTextEx(gFontSemiBold, titulo, (Vector2){r.x + 22, r.y + 12}, 18, 0, WHITE);
    DrawTextEx(gFontSemiBold, TextFormat("%d", valor), (Vector2){r.x + 22, r.y + 50}, 30, 0, WHITE);
}

// ------------------------------------------------------------
// PROGRAMA PRINCIPAL
// ------------------------------------------------------------
int main(void) {

    InitWindow(WINDOW_W, WINDOW_H, "PetControl - Painel");
    SetTargetFPS(60);

    // Tentar carregar Poppins (duas opções de caminho para cobrir builds)
    const char *poppinsRegularPaths[] = {
        "../assets/fonts/Poppins-Regular.ttf",
        "assets/fonts/Poppins-Regular.ttf",
        "../assets/Poppins-Regular.ttf",
        "assets/Poppins-Regular.ttf",
        NULL
    };
    const char *poppinsSemiPaths[] = {
        "../assets/fonts/Poppins-SemiBold.ttf",
        "assets/fonts/Poppins-SemiBold.ttf",
        "../assets/Poppins-SemiBold.ttf",
        "assets/Poppins-SemiBold.ttf",
        NULL
    };
    const char *foundReg = NULL;
    const char *foundSemi = NULL;
    for (int i=0; poppinsRegularPaths[i]; i++) {
        if (FileExists(poppinsRegularPaths[i])) { foundReg = poppinsRegularPaths[i]; break; }
    }
    for (int i=0; poppinsSemiPaths[i]; i++) {
        if (FileExists(poppinsSemiPaths[i])) { foundSemi = poppinsSemiPaths[i]; break; }
    }

    if (foundReg) {
        // carregar com tamanho "base" grande para ficar nítido ao escalar no DrawTextEx
        gFontRegular = LoadFontEx(foundReg, 48, 0, 0);
        SetTextureFilter(gFontRegular.texture, TEXTURE_FILTER_BILINEAR);
    } else {
        TraceLog(LOG_WARNING, "Poppins-Regular não encontrada, usando GetFontDefault()");
        gFontRegular = GetFontDefault();
    }

    if (foundSemi) {
        gFontSemiBold = LoadFontEx(foundSemi, 48, 0, 0);
        SetTextureFilter(gFontSemiBold.texture, TEXTURE_FILTER_BILINEAR);
    } else {
        // fallback: usar mesma regular se semibold não disponível
        gFontSemiBold = gFontRegular;
    }

    // Logo (opcional)
    Texture2D logo = {0};
    if (FileExists("assets/logo.png"))
        logo = LoadTexture("assets/logo.png");
    else if (FileExists("../assets/logo.png"))
        logo = LoadTexture("../assets/logo.png");

    // Carregar clientes da API (inicial)
    carregarClientesAPI();

    int visibleRows = TABLE_VISIBLE_H / ROW_HEIGHT;
    if (visibleRows < 1) visibleRows = 1;

    double lastAutoRefresh = GetTime();
    const double AUTO_REFRESH_INTERVAL = 10.0;

    while (!WindowShouldClose()) {
        
        double now = GetTime();
        if (now - lastAutoRefresh > AUTO_REFRESH_INTERVAL) {
            carregarClientesAPI();
            lastAutoRefresh = now;
        }

        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) scrollY -= wheel * scrollSpeed;

        // total content and max scroll (necessário para scrollbar/pagination)
        float contentH = (float)totalClientes * ROW_HEIGHT;
        float maxScroll = contentH - visibleRows * ROW_HEIGHT;
        if (maxScroll < 0) maxScroll = 0.0f;

        scrollY = clampf(scrollY, 0.0f, maxScroll);

        BeginDrawing();

        // Fundo degradê verde suave
        for (int y = 0; y < WINDOW_H; y++) {
            float t = y / (float)WINDOW_H;
            Color c = ColorLerp(
                (Color){249,252,250,255},
                (Color){241,248,244,255},
                t
            );
            DrawLine(0, y, WINDOW_W, y, c);
        }

        // Top bar
        for (int i = 0; i < WINDOW_W; i++) {
            float t = i / (float)WINDOW_W;
            Color c = ColorLerp(
                (Color){48,160,118,255},
                (Color){30,130,95,255},
                t
            );
            DrawLine(i, 0, i, TOP_BAR_H, c);
        }

        if (logo.id != 0) {
            float maxH = 48.0f;
            float scale = maxH / (float)logo.height;
            if (scale <= 0) scale = 1.0f;
            float logoY = (TOP_BAR_H - (logo.height * scale)) / 2.0f;
            DrawTextureEx(logo, (Vector2){18, logoY}, 0.0f, scale, WHITE);
        }

        // Título (SemiBold)
        DrawTextEx(gFontSemiBold, "PetControl - Painel", (Vector2){90, 22}, 26, 1.0f, WHITE);

        // Side menu
        for (int y = TOP_BAR_H; y < WINDOW_H; y++) {
            float t = (y - TOP_BAR_H) / (float)(WINDOW_H - TOP_BAR_H);
            Color c = ColorLerp(
                (Color){250,251,250,255},
                (Color){244,248,243,255},
                t
            );
            DrawLine(0, y, SIDE_MENU_W, y, c);
        }

        DrawRectangle(SIDE_MENU_W-1, TOP_BAR_H+8, 1, WINDOW_H - TOP_BAR_H - 16,
                      (Color){200,210,200,80});

        // MENU BUTTONS
        if (Botao((Rectangle){30, 100, 160, 45}, "Exportar CSV")) {
            exportarRelatorioCSV("relatorio_planos.csv");
        }

        if (Botao((Rectangle){30, 155, 160, 45}, "Recarregar DB")) {
            carregarClientesAPI();
        }

        if (Botao((Rectangle){30, 210, 160, 45}, "Enviar Avisos")) {
            for (int i = 0; i < totalClientes; i++) {
                Cliente *c = &clientes[i];
                int d = diasRestantes(c->vencimento);

                if ((d < 0 || (d >= 0 && d <= 3)) &&
                    email_valido(c->email) &&
                    !emailJaEnviado(c->id))
                {
                    enviarEmailAPI(c);
                }
            }
        }

        if (Botao((Rectangle){30, 265, 160, 45}, "Sair")) {
            break;
        }

        // CARDS
        int ativos = 0, vencer = 0, expirados = 0;
        for (int i = 0; i < totalClientes; i++) {
            int d = diasRestantes(clientes[i].vencimento);
            if (d < 0) expirados++;
            else if (d <= 3) vencer++;
            else ativos++;
        }

        Card((Rectangle){250, 90, 220, 110}, "Ativos",    ativos,    (Color){83,197,140,255});
        Card((Rectangle){500, 90, 220, 110}, "A Vencer",  vencer,    (Color){253,182,80,255});
        Card((Rectangle){750, 90, 220, 110}, "Expirados", expirados, (Color){245,107,107,255});

        // HEADER DA TABELA (SemiBold)
        DrawTextEx(gFontSemiBold, "ID",         (Vector2){TABLE_X + 10, 230}, 20, 1.0f, (Color){40,60,50,255});
        DrawTextEx(gFontSemiBold, "Cliente",    (Vector2){TABLE_X + 60, 230}, 20, 1.0f, (Color){40,60,50,255});
        DrawTextEx(gFontSemiBold, "Plano",      (Vector2){TABLE_X + 360,230}, 20, 1.0f, (Color){40,60,50,255});
        DrawTextEx(gFontSemiBold, "Vencimento", (Vector2){TABLE_X + 540,230}, 20, 1.0f, (Color){40,60,50,255});

        DrawRectangle(TABLE_X, TABLE_TOP - 20, TABLE_W, TABLE_VISIBLE_H + 40,
                      (Color){250,250,250,255});

        DrawRectangleLinesEx(
            (Rectangle){TABLE_X, TABLE_TOP-20, TABLE_W, TABLE_VISIBLE_H+40},
            1,
            (Color){200,210,200,40}
        );

        // TABELA — CALCULA LINHAS VISÍVEIS
        int firstRow = (int)(scrollY / ROW_HEIGHT);
        float offsetY = fmodf(scrollY, ROW_HEIGHT);

        int rowsToDraw = visibleRows + 1;
        if (firstRow + rowsToDraw > totalClientes)
            rowsToDraw = totalClientes - firstRow;
        if (rowsToDraw < 0)
            rowsToDraw = 0;

        float y = TABLE_TOP - offsetY;

        for (int i = 0; i < rowsToDraw; i++) {
            int idx = firstRow + i;
            if (idx < 0 || idx >= totalClientes) continue;

            Cliente *c = &clientes[idx];
            int d = diasRestantes(c->vencimento);

            Color statusColor =
                (d < 0)  ? (Color){230,90,90,255}   :
                (d <= 3) ? (Color){255,160,70,255} :
                           (Color){35,130,70,255};

            Color bg = (idx % 2 == 0)
                ? (Color){255,255,255,255}
                : (Color){247,250,247,255};

            Rectangle rowRect = {TABLE_X, y - 5, TABLE_W, ROW_HEIGHT - 5};

            if (CheckCollisionPointRec(GetMousePosition(), rowRect))
                bg = (Color){238,246,238,255};

            DrawRectangleRec(rowRect, bg);

            // Conteúdo tabela (Regular)
            DrawTextEx(gFontRegular, TextFormat("%d", c->id),
                       (Vector2){TABLE_X + 10, y}, 18, 1.0f, (Color){40,60,50,255});

            DrawTextEx(gFontRegular, c->nome,
                       (Vector2){TABLE_X + 60, y}, 18, 1.0f, statusColor);

            DrawTextEx(gFontRegular, c->plano,
                       (Vector2){TABLE_X + 360, y}, 18, 1.0f, (Color){50,70,60,255});

            DrawTextEx(gFontRegular,
                       TextFormat("%s (%d dias)", c->vencimento, d),
                       (Vector2){TABLE_X + 540, y},
                       18, 1.0f,
                       (Color){50,70,60,255});

            y += ROW_HEIGHT;
        }

        if (totalClientes == 0) {
            DrawTextEx(gFontRegular,
                "Nenhum cliente encontrado.",
                (Vector2){TABLE_X + 20, TABLE_TOP + 20},
                20, 1.0f,
                (Color){120,140,120,180});
        }

        // SCROLLBAR
        float trackX = TABLE_X + TABLE_W + 8;
        float trackY = TABLE_TOP;
        float trackH = TABLE_VISIBLE_H;

        DrawRectangle(trackX, trackY, SCROLLBAR_W, trackH, (Color){236,242,236,255});
        DrawRectangleLines(trackX, trackY, SCROLLBAR_W, trackH, (Color){220,230,220,120});

        float thumbH = (contentH <= 0.0f)
            ? trackH
            : ((float)visibleRows * ROW_HEIGHT / contentH) * trackH;

        if (thumbH < 20) thumbH = 20;

        float thumbY = (maxScroll <= 0.0f)
            ? trackY
            : trackY + (scrollY / maxScroll) * (trackH - thumbH);

        Rectangle thumb = {trackX, thumbY, SCROLLBAR_W, thumbH};

        for (int i = 0; i < (int)thumb.height; i++) {
            float t = i / (thumb.height > 0 ? thumb.height : 1);
            Color c = ColorLerp(
                (Color){200,230,200,255},
                (Color){150,210,170,255},
                t
            );
            DrawLine(thumb.x, thumb.y + i, thumb.x + thumb.width, thumb.y + i, c);
        }

        DrawRectangleLinesEx(thumb, 1, (Color){140,180,140,140});

        // PAGINAÇÃO
        if (Botao((Rectangle){250, 650, 110, 35}, "Anterior")) {
            scrollY -= visibleRows * ROW_HEIGHT;
            scrollY = clampf(scrollY, 0, maxScroll);
        }

        if (Botao((Rectangle){370, 650, 110, 35}, "Proxima")) {
            scrollY += visibleRows * ROW_HEIGHT;
            scrollY = clampf(scrollY, 0, maxScroll);
        }

        int currentPage = (int)(scrollY / (visibleRows * ROW_HEIGHT)) + 1;
        int pageCount = (int)((contentH + (visibleRows*ROW_HEIGHT - 1)) / (visibleRows*ROW_HEIGHT));
        if (pageCount < 1) pageCount = 1;

        DrawTextEx(
            gFontRegular,
            TextFormat("Pagina %d / %d", currentPage, pageCount),
            (Vector2){520,655},
            18, 1.0f,
            (Color){120,140,120,180}
        );

        EndDrawing();
    }

    if (logo.id != 0)
        UnloadTexture(logo);

    if (gFontRegular.texture.id != 0)
        UnloadFont(gFontRegular);
    // if semi-bold is different, unload it too
    if (gFontSemiBold.texture.id != 0 && gFontSemiBold.texture.id != gFontRegular.texture.id)
        UnloadFont(gFontSemiBold);

    CloseWindow();
    return 0;
}
