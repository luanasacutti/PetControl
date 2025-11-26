module.exports = (app, db) => {

    // LISTAR CLIENTES
    app.get('/api/clientes', (req, res) => {
        db.all(`
            SELECT 
                id,
                nome,
                email,
                telefone,
                cpf_cnpj,
                plano,
                vencimento
            FROM clientes
        `, [], (err, rows) => {
            if (err) return res.status(500).json({ error: err.message });
            res.json(rows);
        });
    });

    // CADASTRAR CLIENTE — GERANDO VENCIMENTO AUTOMÁTICO
    app.post('/api/clientes', (req, res) => {

        const { nome, email, telefone, cpf_cnpj, plano } = req.body;

        if (!nome || !email || !cpf_cnpj || !plano) {
            return res.status(400).json({ error: "Campos obrigatórios faltando" });
        }

        // GERAR VENCIMENTO PARA +1 MÊS
        const hoje = new Date();
        hoje.setMonth(hoje.getMonth() + 1);
        const vencimento = hoje.toISOString().split("T")[0];

        const sql = `
            INSERT INTO clientes (nome, email, telefone, cpf_cnpj, plano, vencimento)
            VALUES (?, ?, ?, ?, ?, ?)
        `;

        db.run(sql,
            [nome, email, telefone, cpf_cnpj, plano, vencimento],
            function(err) {
                if (err) {
                    console.log("Erro ao cadastrar cliente:", err.message);
                    return res.status(500).json({ error: err.message });
                }

                res.json({
                    id: this.lastID,
                    message: "Cliente cadastrado",
                    vencimentoGerado: vencimento
                });
            }
        );
    });

};
