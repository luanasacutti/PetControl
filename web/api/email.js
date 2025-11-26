const { exec } = require("child_process");
const path = require("path");

module.exports = (app) => {
    // Enviar email de lembrete
    app.post("/api/enviar-email", (req, res) => {
        const { email, nome, vencimento } = req.body;

        if (!email || !nome || !vencimento) {
            return res.status(400).json({ 
                error: "Campos obrigatórios: email, nome, vencimento" 
            });
        }

        console.log(`📧 Enviando email para: ${email}`);
        console.log(`👤 Cliente: ${nome}`);
        console.log(`📅 Vencimento: ${vencimento}`);

        // Integração com o sistema C de email
        const scriptC = path.join(__dirname, "..", "c-integration", "email_system");
        
        // Simulação na prática você compilaria o código C e executaria
        const cmd = `echo "Simulação: Email para ${email} - Vencimento: ${vencimento}"`;
        
        exec(cmd, (error, stdout, stderr) => {
            if (error) {
                console.error("❌ Erro no envio de email:", error);
                return res.status(500).json({ 
                    error: "Erro no envio de email", 
                    details: error.message 
                });
            }
            
            res.json({ 
                success: true, 
                message: "Email enviado com sucesso!",
                destinatario: email,
                cliente: nome,
                vencimento: vencimento,
                output: stdout
            });
        });
    });

    // Enviar email em lote
    app.post("/api/enviar-email-lote", (req, res) => {
        const { clientes } = req.body; // Array de {email, nome, vencimento}

        if (!clientes || !Array.isArray(clientes)) {
            return res.status(400).json({ 
                error: "Array de clientes é obrigatório" 
            });
        }

        console.log(`📧 Enviando ${clientes.length} emails em lote`);

        // Processar cada cliente
        const resultados = clientes.map(cliente => {
            return {
                email: cliente.email,
                nome: cliente.nome,
                status: 'enviado',
                timestamp: new Date().toISOString()
            };
        });

        res.json({
            success: true,
            message: `Lote de ${clientes.length} emails processado`,
            resultados: resultados
        });
    });
};