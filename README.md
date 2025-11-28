🐾 PetControl – Sistema Completo de Gerenciamento para Pet Shops

O PetControl é um ecossistema integrado que conecta:

✔ Aplicativo Desktop (C + Raylib + SQLite)
com
✔ Plataforma Web (Node.js + API REST + HTML/CSS/JS)

Permitindo que o pet shop gerencie seus clientes, planos, vencimentos e visualize tudo de forma clara, rápida e profissional.

🧩 1) PetControl Desktop
🖥 Tecnologias

C (ISO C99)

Raylib 5.0 (interface moderna, responsiva e rápida)

SQLite3 (banco de dados local)

cURL (consumo de API web)

✨ Funcionalidades

🔍 Consulta automática dos cadastros feitos pelo site

🔄 Sincronização via API (GET /api/clientes)

✉️ Envio automático de e-mails para planos próximos ao vencimento

📊 Exportação de relatórios CSV

🗂 Design moderno (Poppins font + UI verde soft)

🗄 Banco SQLite local

🚀 Atualização automática a cada 10 segundos

📁 Estrutura do Projeto — Desktop
PetControl/
├── src/
│   ├── main.c
│   ├── painel_api.c
│   ├── sqlite3.c
│   ├── sqlite3.h
│   ├── build.bat
│   └── assets/
│       ├── logo.png
│       └── fonts/
│           ├── Poppins-Regular.ttf
│           └── Poppins-SemiBold.ttf
│
├── database/
│   └── agendpet.db   (não enviado ao GitHub)
│
├── config/
│   └── send_email.ps1
│
├── .gitignore
└── README.md


📌 Observação:
As pastas raylib/ e mingw-w64/ não vão para o GitHub.
São ferramentas do desenvolvedor e não fazem parte do projeto.

🔧 Como Compilar (Windows)
📌 Dependências necessárias:

Raylib 5.0 (Win64)

GCC WinLibs (mingw-w64)

SQLite3 (já incluso no projeto)

curl.exe instalado em: C:\curl\curl.exe

▶️ Compilação

Dentro da pasta /src, execute:

build.bat


O script gera:

PetControl.exe

🧩 2) PetControl Web
🌐 Tecnologias

Node.js (API)

Express.js (rotas)

SQLite3 (mesmo DB utilizado pelo Desktop)

HTML / CSS / JavaScript

JSON Endpoints

📊 Funcionalidades Web

📅 Cadastro de clientes

🐶 Cadastro de pets

💳 Compra de planos

🔗 Envio dos dados para o Desktop via API

📦 Estrutura escalável

⭐ Interface amigável

📁 Estrutura do Projeto — Web
web/
├── server.js
├── api/
│   ├── clientes.js
│   ├── planos.js
│   └── email.js
├── public/
│   ├── index.html
│   ├── clientes.html
│   ├── planos.html
│   ├── vencimentos.html
│   ├── css/
│   └── js/
└── package.json

🔌 Rotas da API
📥 GET /api/clientes

Retorna lista de clientes em JSON.

📤 POST /api/enviar-email

Dispara o script PowerShell para enviar aviso de vencimento.

📥 GET /api/planos

Lista planos disponíveis.

🚀 Como Rodar a Plataforma Web

Entrar na pasta /web

Instalar dependências:

npm install


Iniciar o servidor:

node server.js


O site abre em:

http://localhost:3000

🧩 Integração Desktop ↔ Web (API REST)

O aplicativo em C:

✔ Faz GET clientes
✔ Converte JSON → Estrutura Cliente
✔ Atualiza interface em tempo real
✔ Envia e-mail via POST /api/enviar-email

Fluxo:

Site → Node API → SQLite DB
Desktop → API → sincroniza automaticamente

👩‍💻 Equipe Desenvolvedora
Nome	Função
Guilherme Almeida	Back-end / Banco de Dados
Luana Sacutti	Full-stack & UI/UX
Maria Eduarda Ferraz	Front-end & Design
📞 Contato

Desenvolvido com ❤️ pela equipe PetControl.
Para dúvidas e contribuições, abra uma issue no GitHub!
