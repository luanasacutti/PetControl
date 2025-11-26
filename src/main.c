#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <time.h>

#define DATABASE_FILE "petcontrol.db"

// Estrutura Cliente
typedef struct {
    int id;
    char nome[100];
    char email[100];
    char telefone[20];
    char cpf_cnpj[20];
    char plano[50];
    char vencimento[11]; // YYYY-MM-DD
} Cliente;

// Função para inicializar o banco de dados
int inicializar_banco() {
    sqlite3 *db;
    char *err_msg = 0;
    
    int rc = sqlite3_open(DATABASE_FILE, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return 0;
    }
    
    // Criar tabela de clientes se não existir
    char *sql = "CREATE TABLE IF NOT EXISTS clientes ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "nome TEXT NOT NULL, "
                "email TEXT, "
                "telefone TEXT, "
                "cpf_cnpj TEXT, "
                "plano TEXT, "
                "vencimento DATE);";
    
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_close(db);
    return 1;
}

// Função para cadastrar cliente
void cadastrar_cliente(const char *nome, const char *email, const char *telefone, 
                      const char *cpf_cnpj, const char *plano, const char *vencimento) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_open(DATABASE_FILE, &db);
    if (rc != SQLITE_OK) {
        printf("{\"success\": false, \"error\": \"Erro ao abrir banco\"}");
        return;
    }
    
    char *sql = "INSERT INTO clientes (nome, email, telefone, cpf_cnpj, plano, vencimento) "
                "VALUES (?, ?, ?, ?, ?, ?);";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        printf("{\"success\": false, \"error\": \"Erro no SQL\"}");
        sqlite3_close(db);
        return;
    }
    
    sqlite3_bind_text(stmt, 1, nome, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, telefone, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, cpf_cnpj, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, plano, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, vencimento, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        printf("{\"success\": true, \"id\": %lld}", sqlite3_last_insert_rowid(db));
    } else {
        printf("{\"success\": false, \"error\": \"Erro ao inserir\"}");
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// Função para listar clientes (retorna JSON)
void listar_clientes() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_open(DATABASE_FILE, &db);
    if (rc != SQLITE_OK) {
        printf("[]");
        return;
    }
    
    char *sql = "SELECT * FROM clientes ORDER BY nome;";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        printf("[]");
        sqlite3_close(db);
        return;
    }
    
    printf("[");
    int first = 1;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) printf(",");
        first = 0;
        
        printf("{");
        printf("\"id\": %d,", sqlite3_column_int(stmt, 0));
        printf("\"nome\": \"%s\",", sqlite3_column_text(stmt, 1));
        printf("\"email\": \"%s\",", sqlite3_column_text(stmt, 2));
        printf("\"telefone\": \"%s\",", sqlite3_column_text(stmt, 3));
        printf("\"cpf_cnpj\": \"%s\",", sqlite3_column_text(stmt, 4));
        printf("\"plano\": \"%s\",", sqlite3_column_text(stmt, 5));
        printf("\"vencimento\": \"%s\"", sqlite3_column_text(stmt, 6));
        printf("}");
    }
    
    printf("]");
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// Função para verificar vencimentos
void verificar_vencimentos() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_open(DATABASE_FILE, &db);
    if (rc != SQLITE_OK) {
        printf("[]");
        return;
    }
    
    // Clientes com vencimento nos próximos 7 dias
    char *sql = "SELECT * FROM clientes WHERE date(vencimento) BETWEEN date('now') AND date('now', '+7 days') ORDER BY vencimento;";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        printf("[]");
        sqlite3_close(db);
        return;
    }
    
    printf("[");
    int first = 1;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) printf(",");
        first = 0;
        
        printf("{");
        printf("\"id\": %d,", sqlite3_column_int(stmt, 0));
        printf("\"nome\": \"%s\",", sqlite3_column_text(stmt, 1));
        printf("\"email\": \"%s\",", sqlite3_column_text(stmt, 2));
        printf("\"plano\": \"%s\",", sqlite3_column_text(stmt, 5));
        printf("\"vencimento\": \"%s\"", sqlite3_column_text(stmt, 6));
        printf("}");
    }
    
    printf("]");
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// Função para enviar email (simulação)
void enviar_email(const char *email, const char *nome, const char *vencimento) {
    // Simulação de envio de email
    // Em produção, integraria com libcurl ou sistema de email
    
    FILE *log = fopen("emails_enviados.log", "a");
    if (log) {
        time_t now = time(NULL);
        fprintf(log, "[%s] Email para: %s <%s> - Vencimento: %s\n", 
                ctime(&now), nome, email, vencimento);
        fclose(log);
    }
    
    printf("{\"success\": true, \"message\": \"Email agendado para %s\"}", email);
}

// Função para exportar CSV
void exportar_csv() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_open(DATABASE_FILE, &db);
    if (rc != SQLITE_OK) {
        printf("{\"success\": false}");
        return;
    }
    
    FILE *csv = fopen("clientes_export.csv", "w");
    if (!csv) {
        printf("{\"success\": false}");
        sqlite3_close(db);
        return;
    }
    
    fprintf(csv, "ID,Nome,Email,Telefone,CPF/CNPJ,Plano,Vencimento\n");
    
    char *sql = "SELECT * FROM clientes;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    
    if (rc == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            fprintf(csv, "%d,\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"\n",
                   sqlite3_column_int(stmt, 0),
                   sqlite3_column_text(stmt, 1),
                   sqlite3_column_text(stmt, 2),
                   sqlite3_column_text(stmt, 3),
                   sqlite3_column_text(stmt, 4),
                   sqlite3_column_text(stmt, 5),
                   sqlite3_column_text(stmt, 6));
        }
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    fclose(csv);
    
    printf("{\"success\": true, \"file\": \"clientes_export.csv\"}");
}

int main(int argc, char *argv[]) {
    // Inicializar banco
    inicializar_banco();
    
    if (argc < 2) {
        printf("{\"error\": \"Comando não especificado\"}");
        return 1;
    }
    
    char *comando = argv[1];
    
    if (strcmp(comando, "listar_clientes") == 0) {
        listar_clientes();
    }
    else if (strcmp(comando, "cadastrar_cliente") == 0 && argc == 8) {
        cadastrar_cliente(argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);
    }
    else if (strcmp(comando, "enviar_email") == 0 && argc == 5) {
        enviar_email(argv[2], argv[3], argv[4]);
    }
    else if (strcmp(comando, "verificar_vencimentos") == 0) {
        verificar_vencimentos();
    }
    else if (strcmp(comando, "exportar_csv") == 0) {
        exportar_csv();
    }
    else {
        printf("{\"error\": \"Comando inválido\"}");
    }
    
    return 0;
}
