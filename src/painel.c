// painel.c
// Tema: Verde Soft — UI modernizada (cards com sombra, topo em gradiente,
// botões com sombra, linhas de tabela suaves, thumb da scrollbar estilizado)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "raylib.h"
#include "sqlite3.h"

#define MAX_REG 1024

// Layout constants
#define WINDOW_W 1100
#define WINDOW_H 700
#define TOP_BAR_H 70
#define SIDE_MENU_W 220
#define ROW_HEIGHT 45
#define TABLE_X 240
#define TABLE_W 840
#define TABLE_TOP 265
#define TABLE_VISIBLE_H 360   // height area for visible rows (adjust as needed)
#define SCROLLBAR_W 10

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
    char vencimento[16]; // YYYY-MM-DD
} Cliente;

static Cliente clientes[MAX_REG];
static int totalClientes = 0;

// Scroll state
static float scrollY = 0.0f;        // pixels scrolled from top of table
static float scrollSpeed = 30.0f;   // pixels per wheel tick

// Fontes (Poppins)
static Font gFont = { 0 };
static Font gFontBold = { 0 };

// ------------------------------------------------------------
// Helpers: clamp, safe string copy, color lerp, fade
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

static Color FadeCustom(Color c, float f) {
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
    if (sscanf(dataVenc, "%d-%d-%d", &ano, &mes, &dia) != 3) return 99999;

    struct tm tmv = {0};
    tmv.tm_year = ano - 1900;
    tmv.tm_mon = mes - 1;
    tmv.tm_mday = dia;
    tmv.tm_hour = 12;

    time_t t_venc = mktime(&tmv);
    if (t_venc == (time_t)-1) return 99999;
    time_t now = time(NULL);

    double diff = difftime(t_venc, now);
    int dias = (int)(diff / 86400.0);
    return dias;
}

// ------------------------------------------------------------
// CSV escape (duplica aspas)
// ------------------------------------------------------------
static void csv_escape_and_write(FILE *f, const char *s) {
    if (!f) return;
    if (!s) { fputs("\"\"", f); return; }
    fputc('"', f);
    for (const char *p = s; *p; ++p) {
        if (*p == '"') fputc('"', f); // duplica aspas
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
    if (!f) return 0; // arquivo inexistente => não enviado

    int idl;
    char data[64];
    while (fscanf(f, "%d;%63s", &idl, data) == 2) {
        if (idl == id) {
            fclose(f);
            return 1; // já enviado
        }
    }
    fclose(f);
    return 0;
}

// ------------------------------------------------------------
// LOG: registrar envio
// ------------------------------------------------------------
void registrarEnvioEmail(int id) {
    FILE *f = fopen("email_enviados.log", "a");
    if (!f) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char data[32];
    sprintf(data, "%04d-%02d-%02d",
        t->tm_year + 1900,
        t->tm_mon + 1,
        t->tm_mday
    );

    fprintf(f, "%d;%s\n", id, data);
    fclose(f);
}

// ------------------------------------------------------------
// Sanitize argumento para PowerShell (remove aspas duplas)
// substitui " por ' para evitar quebrar a linha de comando
// ------------------------------------------------------------
static void sanitize_for_powershell(const char *in, char *out, size_t out_sz) {
    if (!out || out_sz == 0) return;
    if (!in) { out[0] = '\0'; return; }
    size_t j = 0;
    for (size_t i = 0; in[i] != '\0' && j + 1 < out_sz; ++i) {
        char ch = in[i];
        if (ch == '"') {
            if (j + 2 < out_sz) { out[j++] = '\''; } // convert quote to single quote
        } else {
            out[j++] = ch;
        }
    }
    out[j] = '\0';
}

// ------------------------------------------------------------
// Enviar e-mail chamando PowerShell
// ------------------------------------------------------------
void enviarEmailCliente(Cliente *c, int dias) {
    if (!c) return;
    if (!email_valido(c->email)) return;
    if (emailJaEnviado(c->id)) return;

    // montar comando com argumentos sanitizados
    char email_s[256], nome_s[256], venc_s[64];
    sanitize_for_powershell(c->email, email_s, sizeof(email_s));
    sanitize_for_powershell(c->nome, nome_s, sizeof(nome_s));
    sanitize_for_powershell(c->vencimento, venc_s, sizeof(venc_s));

    char cmd[1024];
    // certifique-se do caminho correto do script: config/send_email.ps1
    snprintf(cmd, sizeof(cmd),
        "powershell -ExecutionPolicy Bypass -File \"config/send_email.ps1\""
        " -email \"%s\" -nome \"%s\" -vencimento \"%s\" -dias \"%d\"",
        email_s, nome_s, venc_s, dias
    );

    // Run (blocking): se preferir rodar async, trocar por criação de thread
    int rc = system(cmd);
    (void)rc;

    registrarEnvioEmail(c->id);
}

// ------------------------------------------------------------
// Exportar CSV (UTF-8 BOM para compatibilidade web)
// ------------------------------------------------------------
void exportarRelatorioCSV(const char *fname) {
    FILE *f = fopen(fname, "w");
    if (!f) {
        TraceLog(LOG_WARNING, "Não foi possível criar CSV: %s", fname);
        return;
    }

    // BOM
    unsigned char bom[] = {0xEF, 0xBB, 0xBF};
    fwrite(bom, 1, sizeof(bom), f);

    fprintf(f, "ID,Nome,Email,Telefone,CPF_CNPJ,Plano,Vencimento,Situacao,Dias\n");

    for (int i = 0; i < totalClientes; i++) {
        Cliente *c = &clientes[i];
        int dias = diasRestantes(c->vencimento);

        const char *sit =
            (dias < 0) ? "Expirado" :
            (dias <= 3 ? "A vencer" : "Ativo");

        // Escapar corretamente
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
    TraceLog(LOG_INFO, "Relatorio gerado: %s", fname);
}

// ------------------------------------------------------------
// Botão estilizado (Verde Soft) — usa gFont
// ------------------------------------------------------------
bool Botao(Rectangle r, const char *label) {
    Vector2 m = GetMousePosition();
    bool hover = CheckCollisionPointRec(m, r);
    bool pressed = IsMouseButtonDown(MOUSE_LEFT_BUTTON) && hover;

    Color base = (Color){244, 250, 246, 255}; // very light greenish
    Color baseHover = (Color){235, 246, 240, 255};
    Color border = (Color){180, 200, 190, 180};
    Color shadow = (Color){0,0,0,32};
    Color textCol = (Color){32,56,44,255};

    // sombra
    DrawRectangleRounded((Rectangle){r.x+2, r.y+3, r.width, r.height}, 0.16f, 6, shadow);

    DrawRectangleRounded(r, 0.16f, 6, hover ? baseHover : base);
    DrawRectangleRoundedLines(r, 0.16f, 6, 1.0f, border);

    int fontSize = 18;
    Vector2 txtSize = MeasureTextEx(gFont.texture.id ? gFont : GetFontDefault(), label, fontSize, 0);
    int tw = (int)txtSize.x;
    // efeito "pressionado"
    float offY = pressed ? 2.0f : 0.0f;
    DrawTextEx(gFont.texture.id ? gFont : GetFontDefault(), label, (Vector2){ r.x + (r.width - txtSize.x)/2.0f, r.y + (r.height - txtSize.y)/2.0f + offY }, fontSize, 0, textCol);

    return hover && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
}

// ------------------------------------------------------------
// Card Bonito (Verde Soft) — usa gFont
// ------------------------------------------------------------
void Card(Rectangle r, const char *titulo, int valor, Color cor) {
    Color shadow = (Color){0,0,0,40};
    // sombra
    DrawRectangleRounded((Rectangle){r.x+4, r.y+6, r.width, r.height}, 0.22f, 12, shadow);

    // leve brilho no topo do card
    Color topBright = ColorLerp(cor, (Color){255,255,255,255}, 0.12f);

    DrawRectangleRounded(r, 0.22f, 12, cor);
    // highlight
    DrawRectangleRounded((Rectangle){r.x, r.y, r.width, r.height*0.34f}, 0.22f, 12, FadeCustom(topBright, 0.22f));


    // titulo
    DrawTextEx(gFontBold.texture.id ? gFontBold : (gFont.texture.id ? gFont : GetFontDefault()),
               titulo,
               (Vector2){r.x + 22, r.y + 12},
               18, 0, WHITE);

    // valor
    DrawTextEx(gFontBold.texture.id ? gFontBold : (gFont.texture.id ? gFont : GetFontDefault()),
               TextFormat("%d", valor),
               (Vector2){r.x + 22, r.y + 56},
               34, 0, WHITE);
}

// ------------------------------------------------------------
// Carregar DB (com envio automático) - igual ao seu original
// ------------------------------------------------------------
int carregarClientesDB(const char *dbfile) {
    sqlite3 *db;
    sqlite3_stmt *stmt;

    const char *sql =
        "SELECT id, nome, email, telefone, cpf_cnpj, plano, vencimento "
        "FROM clientes ORDER BY id;";

    if (sqlite3_open(dbfile, &db) != SQLITE_OK) {
        TraceLog(LOG_WARNING, "Falha ao abrir DB: %s", dbfile);
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        TraceLog(LOG_WARNING, "Falha prepare: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }

    totalClientes = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (totalClientes >= MAX_REG) break;

        Cliente *c = &clientes[totalClientes];
        c->id = sqlite3_column_int(stmt, 0);

        const unsigned char *t;
        t = sqlite3_column_text(stmt, 1); safe_strcpy(c->nome, t ? (const char*)t : "", sizeof(c->nome));
        t = sqlite3_column_text(stmt, 2); safe_strcpy(c->email, t ? (const char*)t : "", sizeof(c->email));
        t = sqlite3_column_text(stmt, 3); safe_strcpy(c->telefone, t ? (const char*)t : "", sizeof(c->telefone));
        t = sqlite3_column_text(stmt, 4); safe_strcpy(c->cpf_cnpj, t ? (const char*)t : "", sizeof(c->cpf_cnpj));
        t = sqlite3_column_text(stmt, 5); safe_strcpy(c->plano, t ? (const char*)t : "", sizeof(c->plano));
        t = sqlite3_column_text(stmt, 6); safe_strcpy(c->vencimento, t ? (const char*)t : "", sizeof(c->vencimento));

        // Envio automático: se vencido ou a vencer (<= 3 dias)
        int d = diasRestantes(c->vencimento);
        if ((d < 0 || d <= 3) && email_valido(c->email) && !emailJaEnviado(c->id)) {
            enviarEmailCliente(c, d);
        }

        totalClientes++;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return totalClientes;
}

// ------------------------------------------------------------
// Programa principal (PetControl - painel.c)
// ------------------------------------------------------------
int main(void) {
    // Inicializa janela
    InitWindow(WINDOW_W, WINDOW_H, "PetControl - Painel");
    SetTargetFPS(60);

    // Carrega Poppins a partir de assets/fonts/
    const char *poppins_regular = "assets/fonts/Poppins-Regular.ttf";
    const char *poppins_semibold = "assets/fonts/Poppins-SemiBold.ttf";

    if (FileExists(poppins_regular)) {
        gFont = LoadFontEx(poppins_regular, 22, 0, 0);
        TraceLog(LOG_INFO, "Fonte carregada: %s", poppins_regular);
    } else {
        TraceLog(LOG_WARNING, "Fonte Poppins-Regular não encontrada em %s — usando fonte padrão.", poppins_regular);
        gFont = GetFontDefault();
    }

    if (FileExists(poppins_semibold)) {
        gFontBold = LoadFontEx(poppins_semibold, 26, 0, 0);
        TraceLog(LOG_INFO, "Fonte semibold carregada: %s", poppins_semibold);
    } else {
        // se semibold não existir, reusar a regular (ou default)
        if (gFont.texture.id) {
            gFontBold = gFont;
        } else {
            gFontBold = GetFontDefault();
        }
        TraceLog(LOG_WARNING, "Fonte Poppins-SemiBold não encontrada em %s — fallback aplicado.", poppins_semibold);
    }

    // Carrega recursos
    Texture2D logo = {0};
    if (FileExists("logo.png")) {
        logo = LoadTexture("logo.png");
    }

    // Carrega DB (arquivo agendpet.db)
    int loaded = carregarClientesDB("database/agendpet.db");
    if (!loaded) {
        TraceLog(LOG_WARNING, "Nenhum cliente carregado. Verifique agendpet.db e tabela clientes.");
    }

    int page = 0;
    const int perPage = 14;

    // Pre-calc visible rows
    int visibleRows = (int)(TABLE_VISIBLE_H / ROW_HEIGHT);
    if (visibleRows < 1) visibleRows = 1;

    double lastAutoRefresh = 0.0;
    const double AUTO_REFRESH_INTERVAL = 10.0; // segundos

    while (!WindowShouldClose()) {
        // periodic auto-refresh
        double now = GetTime();
        if (now - lastAutoRefresh > AUTO_REFRESH_INTERVAL) {
            carregarClientesDB("database/agendpet.db");
            lastAutoRefresh = now;
        }

        // Handle input for scroll (mouse wheel)
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            scrollY -= wheel * scrollSpeed; // wheel positive when scroll up
        }

        // total content height (necessário antes de desenhar scrollbar)
        float contentH = (float)totalClientes * (float)ROW_HEIGHT;
        float maxScroll = contentH - (float)visibleRows * (float)ROW_HEIGHT;
        if (maxScroll < 0) maxScroll = 0;

        // Clamp scroll
        scrollY = clampf(scrollY, 0.0f, maxScroll);

        // Begin drawing
        BeginDrawing();

        // Background - suave verde claro com leve gradiente
        for (int y = 0; y < WINDOW_H; y++) {
            float t = y / (float)WINDOW_H;
            Color top = (Color){250,252,250,255};    // quase branco esverdeado
            Color bot = (Color){243,249,244,255};    // verde suave
            Color c = ColorLerp(top, bot, t);
            DrawLine(0, y, WINDOW_W, y, c);
        }

        // ------------------------------------------------------------
        // Barra superior com logotipo e título (gradiente verde)
        // ------------------------------------------------------------
        for (int i = 0; i < WINDOW_W; i++) {
            float t = i / (float)WINDOW_W;
            Color a = (Color){82,196,143,255}; // mais suave
            Color b = (Color){45,150,100,255};
            Color cc = ColorLerp(a, b, t);
            DrawLine(i, 0, i, TOP_BAR_H, cc);
        }

        if (logo.id != 0) {
            float maxLogoHeight = 48.0f; // ajustar
            float scale = maxLogoHeight / (float)logo.height;
            if (scale <= 0) scale = 1.0f;
            float logoY = (TOP_BAR_H - (logo.height * scale)) / 2.0f;
            DrawTextureEx(logo, (Vector2){18, logoY}, 0.0f, scale, WHITE);
        }

        // Título usa gFontBold (fallback para gFont/default se necessário)
        DrawTextEx(gFontBold.texture.id ? gFontBold : (gFont.texture.id ? gFont : GetFontDefault()),
                   "PetControl - Painel",
                   (Vector2){90, 22},
                   26, 0, WHITE);

        // ------------------------------------------------------------
        // MENU LATERAL (Suave)
        // ------------------------------------------------------------
        for (int y = TOP_BAR_H; y < WINDOW_H; y++) {
            float t = (y - TOP_BAR_H) / (float)(WINDOW_H - TOP_BAR_H);
            Color a = (Color){250,251,250,255};
            Color b = (Color){246,249,247,255};
            Color cc = ColorLerp(a, b, t);
            DrawLine(0, y, SIDE_MENU_W, y, cc);
        }
        DrawRectangle(SIDE_MENU_W-1, TOP_BAR_H+8, 1, WINDOW_H - TOP_BAR_H - 16, (Color){200,210,200,60});

        // Botões no menu lateral
        if (Botao((Rectangle){30, 100, 160, 45}, "Exportar CSV")) {
            exportarRelatorioCSV("relatorio_planos.csv");
        }
        if (Botao((Rectangle){30, 155, 160, 45}, "Recarregar DB")) {
            carregarClientesDB("database/agendpet.db");
        }
        if (Botao((Rectangle){30, 210, 160, 45}, "Enviar Avisos")) {
            for (int i = 0; i < totalClientes; i++) {
                Cliente *c = &clientes[i];
                int d = diasRestantes(c->vencimento);
                if ((d < 0 || d <= 3) && email_valido(c->email) && !emailJaEnviado(c->id)) {
                    enviarEmailCliente(c, d);
                }
            }
        }
        if (Botao((Rectangle){30, 265, 160, 45}, "Sair")) {
            break;
        }

        // ------------------------------------------------------------
        // CARDS (Planos Ativos / A Vencer / Expirados)
        // ------------------------------------------------------------
        int ativos = 0, vencer = 0, expirados = 0;
        for (int i = 0; i < totalClientes; i++) {
            int d = diasRestantes(clientes[i].vencimento);
            if (d < 0) expirados++;
            else if (d <= 3) vencer++;
            else ativos++;
        }

        Card((Rectangle){250, 90, 220, 110}, "Planos Ativos", ativos, (Color){120,200,150,255});
        Card((Rectangle){500, 90, 220, 110}, "A Vencer", vencer, (Color){253,182,80,255});
        Card((Rectangle){750, 90, 220, 110}, "Expirados", expirados, (Color){245,107,107,255});

        // ------------------------------------------------------------
        // CABEÇALHO TABELA
        // ------------------------------------------------------------
        DrawTextEx(gFont.texture.id ? gFont : GetFontDefault(), "ID", (Vector2){TABLE_X + 10, 230}, 20, 0, (Color){40,60,50,255});
        DrawTextEx(gFont.texture.id ? gFont : GetFontDefault(), "Cliente", (Vector2){TABLE_X + 60, 230}, 20, 0, (Color){40,60,50,255});
        DrawTextEx(gFont.texture.id ? gFont : GetFontDefault(), "Plano", (Vector2){TABLE_X + 360, 230}, 20, 0, (Color){40,60,50,255});
        DrawTextEx(gFont.texture.id ? gFont : GetFontDefault(), "Vencimento", (Vector2){TABLE_X + 540, 230}, 20, 0, (Color){40,60,50,255});

        // ------------------------------------------------------------
        // TABELA COM ROLAGEM (estilizada)
        // ------------------------------------------------------------
        int firstRow = (int)(scrollY / ROW_HEIGHT);
        float offsetY = fmodf(scrollY, ROW_HEIGHT);

        int rowsToDraw = visibleRows + 1;
        if (firstRow + rowsToDraw > totalClientes) rowsToDraw = totalClientes - firstRow;

        DrawRectangle(TABLE_X, TABLE_TOP - 20, TABLE_W, TABLE_VISIBLE_H + 40, (Color){255,255,255,255});
        DrawRectangleLinesEx((Rectangle){TABLE_X, TABLE_TOP-20, TABLE_W, TABLE_VISIBLE_H+40}, 1, (Color){220,230,220,60});

        float y = TABLE_TOP - offsetY;
        for (int i = 0; i < rowsToDraw; i++) {
            int idx = firstRow + i;
            if (idx < 0 || idx >= totalClientes) continue;
            Cliente *c = &clientes[idx];
            int d = diasRestantes(c->vencimento);

            Color statusColor = (d < 0) ? (Color){200,70,70,255} :
                                (d <= 3) ? (Color){230,140,60,255} :
                                           (Color){34,120,70,255};

            Color bg = (idx % 2 == 0) ? (Color){255,255,255,255} : (Color){248,251,248,255};
            Rectangle rowRect = {TABLE_X, y - 5, TABLE_W, ROW_HEIGHT - 5};
            if (CheckCollisionPointRec(GetMousePosition(), rowRect)) {
                bg = (Color){240,249,238,255};
            }
            DrawRectangleRec(rowRect, bg);

            DrawTextEx(gFont.texture.id ? gFont : GetFontDefault(), TextFormat("%d", c->id), (Vector2){TABLE_X + 10, y}, 18, 0, (Color){40,60,50,255});
            DrawTextEx(gFont.texture.id ? gFont : GetFontDefault(), c->nome, (Vector2){TABLE_X + 60, y}, 18, 0, statusColor);
            DrawTextEx(gFont.texture.id ? gFont : GetFontDefault(), c->plano, (Vector2){TABLE_X + 360, y}, 18, 0, (Color){50,70,60,255});
            DrawTextEx(gFont.texture.id ? gFont : GetFontDefault(), TextFormat("%s (%d dias)", c->vencimento, d), (Vector2){TABLE_X + 540, y}, 18, 0, (Color){60,80,70,255});

            y += ROW_HEIGHT;
        }

        if (totalClientes == 0) {
            DrawTextEx(gFont.texture.id ? gFont : GetFontDefault(), "Nenhum cliente encontrado.", (Vector2){TABLE_X + 20, TABLE_TOP + 20}, 20, 0, (Color){120,140,120,160});
        }

        // ------------------------------------------------------------
        // Scrollbar visual (direita da tabela)
        // ------------------------------------------------------------
        float trackX = TABLE_X + TABLE_W + 12;
        float trackY = TABLE_TOP;
        float trackH = TABLE_VISIBLE_H;
        DrawRectangle((int)trackX, (int)trackY, SCROLLBAR_W, (int)trackH, (Color){241,248,238,255});
        DrawRectangleLines((int)trackX, (int)trackY, SCROLLBAR_W, (int)trackH, (Color){220,230,220,100});

        float thumbH;
        if (contentH <= 0.0f) thumbH = trackH;
        else thumbH = ( (float)visibleRows * ROW_HEIGHT / contentH ) * trackH;
        if (thumbH < 28.0f) thumbH = 28.0f; // min thumb size

        float thumbY;
        if (maxScroll <= 0.0f) thumbY = trackY;
        else thumbY = trackY + (scrollY / maxScroll) * (trackH - thumbH);

        Rectangle thumb = {(int)trackX, (int)thumbY, SCROLLBAR_W, (int)thumbH};
        // thumb gradient vertical
        for (int i = 0; i < (int)thumb.height; i++) {
            float t = i / (thumb.height > 0 ? thumb.height : 1);
            Color c = ColorLerp((Color){200,230,200,255}, (Color){160,210,170,255}, t);
            DrawLine(thumb.x, thumb.y + i, thumb.x + thumb.width, thumb.y + i, c);
        }
        DrawRectangleLinesEx(thumb, 1, (Color){140,180,140,160});

        // ------------------------------------------------------------
        // Paginação (botões)
        // ------------------------------------------------------------
        if (Botao((Rectangle){250, 650, 110, 35}, "Anterior")) {
            scrollY -= (float)visibleRows * ROW_HEIGHT;
            scrollY = clampf(scrollY, 0.0f, maxScroll);
        }
        if (Botao((Rectangle){370, 650, 110, 35}, "Próxima")) {
            scrollY += (float)visibleRows * ROW_HEIGHT;
            scrollY = clampf(scrollY, 0.0f, maxScroll);
        }

        int currentPage = (int)(scrollY / ((float)visibleRows * ROW_HEIGHT)) + 1;
        int pageCount = (int)( (contentH + (visibleRows*ROW_HEIGHT - 1)) / (visibleRows*ROW_HEIGHT) );
        if (pageCount < 1) pageCount = 1;
        DrawTextEx(gFont.texture.id ? gFont : GetFontDefault(), TextFormat("Página %d / %d", currentPage, pageCount), (Vector2){520,655}, 18, 0, (Color){110,130,110,160});

        EndDrawing();
    }

    // Cleanup
    if (logo.id != 0) UnloadTexture(logo);

    // Unload fonts only if loaded from file (avoid double unload if gFontBold == gFont)
    if (gFont.texture.id && strcmp(gFont.glyphs ? "": "", "")==0) {
        // can't reliably check source, so unload if texture id nonzero and not default
        UnloadFont(gFont);
    }
    if (gFontBold.texture.id && gFontBold.texture.id != gFont.texture.id) {
        UnloadFont(gFontBold);
    }

    CloseWindow();
    return 0;
}
