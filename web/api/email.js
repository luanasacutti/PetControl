// /api/enviar-email
// Espera JSON:
// { "email": "...", "nome": "...", "vencimento": "2025-02-15" }

const { exec } = require("child_process");
const path = require("path");

module.exports = (app) => {

    app.post("/api/enviar-email", (req, res) => {

        console.log("📩 Recebido POST em /api/enviar-email");
        console.log("BODY RECEBIDO:", req.body);

        const { email, nome, vencimento } = req.body;

        if (!email || !nome || !vencimento) {
            console.log("❌ ERRO: Campos faltando no JSON!");
            return res.status(400).json({
                error: "Campos obrigatórios: email, nome, vencimento"
            });
        }

        const script = path.join(__dirname, "..", "..", "config", "send_email.ps1");

        const cmd = `powershell -ExecutionPolicy Bypass -File "${script}" -email "${email}" -nome "${nome}" -vencimento "${vencimento}"`;

        console.log("🔧 Executando PowerShell:");
        console.log(cmd);

        exec(cmd, (error, stdout, stderr) => {
            if (error) {
                console.error("❌ Erro PowerShell:", error);
                console.error("STDERR:", stderr);
                return res.status(500).json({
                    error: error.message,
                    stderr
                });
            }

            console.log("📨 PowerShell retorno:", stdout);

            res.json({
                ok: true,
                resposta: stdout
            });
        });
    });

};
