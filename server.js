const express = require('express');
const path = require('path');
const fs = require('fs');
const { exec, execSync } = require('child_process');
const app = express();
const PORT = 5000;

// Middleware
app.use(express.json());
app.use(express.static(path.join(__dirname, 'web')));

// Função para executar comandos C e capturar JSON
function executarComandoC(comando) {
    try {
        // Compila e executa o sistema C
        if (!fs.existsSync('sistema_petcontrol.exe')) {
            console.log('🔨 Compilando sistema C...');
            execSync('gcc -o sistema_petcontrol.exe src/main.c src/sqlite3.c -DSQLITE_THREADSAFE=0');
        }
        
        const resultado = execSync(`sistema_petcontrol.exe ${comando}`).toString();
        return JSON.parse(resultado);
    } catch (error) {
        console.error('Erro no sistema C:', error);
        return { error: 'Sistema C indisponível' };
    }
}

// Rotas que usam o sistema C como backend
app.get('/api/clientes', (req, res) => {
    const clientes = executarComandoC('listar_clientes');
    res.json(clientes);
});

app.post('/api/clientes', (req, res) => {
    const { nome, email, telefone, cpf_cnpj, plano, vencimento } = req.body;
    const resultado = executarComandoC(`cadastrar_cliente "${nome}" "${email}" "${telefone}" "${cpf_cnpj}" "${plano}" "${vencimento}"`);
    res.json(resultado);
});

app.post('/api/enviar-email', (req, res) => {
    const { email, nome, vencimento } = req.body;
    const resultado = executarComandoC(`enviar_email "${email}" "${nome}" "${vencimento}"`);
    res.json(resultado);
});

app.get('/api/planos', (req, res) => {
    const planos = executarComandoC('listar_planos');
    res.json(planos);
});

app.get('/api/vencimentos', (req, res) => {
    const vencimentos = executarComandoC('verificar_vencimentos');
    res.json(vencimentos);
});

app.get('/api/exportar-csv', (req, res) => {
    const resultado = executarComandoC('exportar_csv');
    res.json(resultado);
});

// Frontend
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'web', 'index.html'));
});

app.get('/clientes', (req, res) => {
    res.sendFile(path.join(__dirname, 'web', 'clientes.html'));
});

app.get('/planos', (req, res) => {
    res.sendFile(path.join(__dirname, 'web', 'planos.html'));
});

app.get('/vencimentos', (req, res) => {
    res.sendFile(path.join(__dirname, 'web', 'vencimentos.html'));
});

app.listen(PORT, () => {
    console.log('🐾 PETCONTROL - SISTEMA C INTEGRADO 🐾');
    console.log(`📍 Frontend: http://localhost:${PORT}`);
    console.log(`🔧 Backend: Sistema C (SQLite + Email + Vencimentos)`);
    console.log(`📊 API: http://localhost:${PORT}/api/clientes`);
});