module.exports = (app, db) => {
    // GET - Listar todos os clientes
    app.get('/api/clientes', (req, res) => {
        const sql = 'SELECT * FROM clientes ORDER BY id DESC';
        
        db.all(sql, [], (err, rows) => {
            if (err) {
                return res.status(500).json({ 
                    error: 'Erro ao buscar clientes', 
                    details: err.message 
                });
            }
            res.json(rows);
        });
    });

    // GET - Buscar cliente por ID
    app.get('/api/clientes/:id', (req, res) => {
        const id = req.params.id;
        const sql = 'SELECT * FROM clientes WHERE id = ?';
        
        db.get(sql, [id], (err, row) => {
            if (err) {
                return res.status(500).json({ 
                    error: 'Erro ao buscar cliente', 
                    details: err.message 
                });
            }
            if (!row) {
                return res.status(404).json({ error: 'Cliente não encontrado' });
            }
            res.json(row);
        });
    });

    // POST - Criar novo cliente
    app.post('/api/clientes', (req, res) => {
        const { nome, email, telefone, cpf_cnpj, plano, vencimento } = req.body;
        
        if (!nome || !telefone || !plano || !vencimento) {
            return res.status(400).json({ 
                error: 'Campos obrigatórios: nome, telefone, plano, vencimento' 
            });
        }
        
        const sql = `INSERT INTO clientes (nome, email, telefone, cpf_cnpj, plano, vencimento) 
                     VALUES (?, ?, ?, ?, ?, ?)`;
        
        db.run(sql, [nome, email, telefone, cpf_cnpj, plano, vencimento], function(err) {
            if (err) {
                return res.status(500).json({ 
                    error: 'Erro ao cadastrar cliente', 
                    details: err.message 
                });
            }
            res.status(201).json({ 
                id: this.lastID,
                message: 'Cliente cadastrado com sucesso!',
                cliente: { id: this.lastID, nome, email, telefone, plano, vencimento }
            });
        });
    });

    // PUT - Atualizar cliente
    app.put('/api/clientes/:id', (req, res) => {
        const id = req.params.id;
        const { nome, email, telefone, cpf_cnpj, plano, vencimento } = req.body;
        
        const sql = `UPDATE clientes SET nome=?, email=?, telefone=?, cpf_cnpj=?, plano=?, vencimento=?
                     WHERE id=?`;
        
        db.run(sql, [nome, email, telefone, cpf_cnpj, plano, vencimento, id], function(err) {
            if (err) {
                return res.status(500).json({ 
                    error: 'Erro ao atualizar cliente', 
                    details: err.message 
                });
            }
            if (this.changes === 0) {
                return res.status(404).json({ error: 'Cliente não encontrado' });
            }
            res.json({ message: 'Cliente atualizado com sucesso!' });
        });
    });

    // DELETE - Remover cliente
    app.delete('/api/clientes/:id', (req, res) => {
        const id = req.params.id;
        const sql = 'DELETE FROM clientes WHERE id = ?';
        
        db.run(sql, [id], function(err) {
            if (err) {
                return res.status(500).json({ 
                    error: 'Erro ao remover cliente', 
                    details: err.message 
                });
            }
            if (this.changes === 0) {
                return res.status(404).json({ error: 'Cliente não encontrado' });
            }
            res.json({ message: 'Cliente removido com sucesso!' });
        });
    });

    // POST - Agendamento
    app.post('/api/agendamentos', (req, res) => {
        const { cliente_id, pet_nome, servico, data_agendamento } = req.body;
        
        const sql = `INSERT INTO agendamentos (cliente_id, pet_nome, servico, data_agendamento) 
                     VALUES (?, ?, ?, ?)`;
        
        db.run(sql, [cliente_id, pet_nome, servico, data_agendamento], function(err) {
            if (err) {
                return res.status(500).json({ 
                    error: 'Erro ao criar agendamento', 
                    details: err.message 
                });
            }
            res.status(201).json({ 
                id: this.lastID,
                message: 'Agendamento criado com sucesso!'
            });
        });
    });

    // GET - Agendamentos
    app.get('/api/agendamentos', (req, res) => {
        const sql = `
            SELECT a.*, c.nome as cliente_nome 
            FROM agendamentos a 
            LEFT JOIN clientes c ON a.cliente_id = c.id 
            ORDER BY a.data_agendamento DESC
        `;
        
        db.all(sql, [], (err, rows) => {
            if (err) {
                return res.status(500).json({ 
                    error: 'Erro ao buscar agendamentos', 
                    details: err.message 
                });
            }
            res.json(rows);
        });
    });
};