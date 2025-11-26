module.exports = (app, db) => {
    // GET - Listar todos os planos cadastrados nos clientes
    app.get('/api/planos', (req, res) => {
        const sql = `
            SELECT id, nome, email, telefone, cpf_cnpj, plano, vencimento
            FROM clientes
            ORDER BY id DESC
        `;

        db.all(sql, [], (err, rows) => {
            if (err) {
                return res.status(500).json({ 
                    error: 'Erro ao buscar planos', 
                    details: err.message 
                });
            }
            
            // Estatísticas dos planos
            const stats = {
                total: rows.length,
                basico: rows.filter(c => c.plano === 'Básico').length,
                profissional: rows.filter(c => c.plano === 'Profissional').length,
                enterprise: rows.filter(c => c.plano === 'Enterprise').length
            };
            
            res.json({
                clientes: rows,
                estatisticas: stats
            });
        });
    });

    // GET - Estatísticas dos planos
    app.get('/api/planos/estatisticas', (req, res) => {
        const sql = `
            SELECT plano, COUNT(*) as total
            FROM clientes 
            GROUP BY plano
        `;

        db.all(sql, [], (err, rows) => {
            if (err) {
                return res.status(500).json({ 
                    error: 'Erro ao buscar estatísticas', 
                    details: err.message 
                });
            }
            res.json(rows);
        });
    });
};
