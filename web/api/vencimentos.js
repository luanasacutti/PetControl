module.exports = (app, db) => {
    app.get('/api/vencimentos', (req, res) => {
        const days = parseInt(req.query.days) || 7;

        const sql = `
            SELECT id, nome, email, telefone, cpf_cnpj, plano, vencimento
            FROM clientes
            WHERE date(vencimento) BETWEEN date('now') AND date('now', '+' || ? || ' days')
            ORDER BY vencimento ASC
        `;

        db.all(sql, [days], (err, rows) => {
            if (err) {
                return res.status(500).json({ 
                    error: 'Erro ao buscar vencimentos', 
                    details: err.message 
                });
            }
            res.json(rows);
        });
    });

    // GET - Vencimentos hoje
    app.get('/api/vencimentos/hoje', (req, res) => {
        const sql = `
            SELECT id, nome, email, telefone, plano, vencimento
            FROM clientes
            WHERE date(vencimento) = date('now')
            ORDER BY nome
        `;

        db.all(sql, [], (err, rows) => {
            if (err) {
                return res.status(500).json({ 
                    error: 'Erro ao buscar vencimentos de hoje', 
                    details: err.message 
                });
            }
            res.json(rows);
        });
    });
};