🐾 PetControl — Sistema Completo para Pet Shops

Aplicação Desktop + Plataforma Web

O PetControl é um sistema híbrido criado para auxiliar pet shops na organização de clientes, planos, agendamentos e controle financeiro.

Ele é composto por duas plataformas integradas:

PetControl Desktop (C + Raylib + SQLite)

PetControl Web (Node.js + Express + HTML/CSS/JS)

🖥️ 1) PetControl Desktop — Aplicação em C + Raylib

Interface moderna, rápida e responsiva desenvolvida em C, utilizando:

Raylib 5.0 (interface gráfica)

SQLite3 (banco local)

Envio automático de e-mails por PowerShell

Atualização automática dos dados a cada 10 segundos

✨ Funcionalidades

✔️ Consulta dos cadastros feitos pelo site
✔️ Exportação de relatórios em CSV
✔️ Envio automático de e-mails para planos vencidos / próximos do vencimento
✔️ Interface moderna (Poppins + UI verde soft)
✔️ Sistema leve, rápido e totalmente offline
✔️ Banco SQLite incluso no repositório

🧱 Estrutura do Projeto (Desktop)
```text
PetControl/
├── src/
│   ├── painel.c
│   ├── painel_api.c
│   ├── sqlite3.c
│   ├── sqlite3.h
│   ├── build.bat
│   └── main.c (não utilizado)
│
├── assets/
│   ├── logo.png
│   └── fonts/
│       ├── Poppins-Regular.ttf
│       └── Poppins-SemiBold.ttf
│
├── database/
│   └── agendpet.db
│
├── config/
│   └── send_email.ps1
│
└── PetControl.exe   (gerado pelo build)
```

⚙️ Como Compilar o Desktop (Windows)
📦 Dependências obrigatórias

Raylib 5.0 Win64

GCC WinLibs (mingw-w64)

SQLite3 (já incluso)

▶️ Compilar

Abra o terminal na pasta /src e execute:
```text
build.bat
```


Após a compilação, o executável é gerado em:
```text'

../PetControl.exe
```

🌐 2) PetControl Web — Plataforma Online (Node.js + HTML)

Versão web do sistema, com interface responsiva e APIs internas em Node.js.

✨ Funcionalidades

📅 Agenda inteligente
🐶 Cadastro completo de pets
👥 Gestão de clientes
📦 Controle de estoque com alertas
💰 PDV simples
📊 Relatórios
🌎 Interface moderna e responsiva

📂 Estrutura da pasta web/
```text
web/
├── api/
│   ├── clientes.js
│   ├── planos.js
│   ├── vencimentos.js
│   └── email.js
│
├── public/
│   ├── assets/
│   ├── css/
│   ├── js/
│   ├── index.html
│   ├── clientes.html
│   ├── planos.html
│   └── vencimentos.html
│
├── server.js
├── package.json
└── package-lock.json
```

▶️ Como Rodar o PetControl Web (VS Code)
1. Acesse a pasta do projeto:
```text
cd web
```

2. Instale as dependências:
```text
npm install
```

3. Execute o servidor:

```text
node server.js
```

4. Acesse no navegador:

👉 http://localhost:3000

🌎 Versão Online (GitHub Pages)

Você também pode acessar a versão web diretamente:

👉 https://luanasacutti.github.io/PetControl/

## 👩‍💻 Equipe Desenvolvedora

| Nome                  | Função                |
|----------------------|------------------------|
| **Guilherme Almeida** | Back-end & Database    |
| **Luana Sacutti**     | Full-stack Development |
| **Maria Eduarda Ferraz** | Front-end & Design |

💌 Contato

Desenvolvido com ❤️ pela equipe PetControl.
Para dúvidas ou feedbacks, entre em contato pelo GitHub.
