🐾 PetControl — Sistema de Gestão

Sistema completo para pet shops, composto por duas plataformas integradas:
PetControl Desktop (C + Raylib + SQLite) e PetControl Web (Java + MySQL + Front-end).

🧩 1) PetControl Desktop — Gerenciamento de Planos

Aplicação desenvolvida em C + Raylib + SQLite, com atualização automática e visual moderno.

✨ Funcionalidades

🔍 Consulta de cadastros feitos pelo site

📊 Exportação de relatórios em CSV

✉️ Envio automático de e-mails de aviso para planos próximos do vencimento

🗄 Banco de dados local SQLite3

🔄 Atualização automática a cada 10 segundos

🎨 Design moderno (Fonte Poppins + UI verde soft)

📁 Estrutura do Projeto (Desktop)
PetControl/
├── src/
│   ├── main.c
│   ├── painel_api.c
│   ├── sqlite3.c
│   ├── sqlite3.h
│   ├── build.bat
│
├── assets/
│   ├── logo.png
│   └── fonts/
│       ├── Poppins-Regular.ttf
│       └── Poppins-SemiBold.ttf
│
├── database/
│   └── agendpet.db        (não enviado ao GitHub)
│
├── config/
│   └── send_email.ps1     (opcional)
│
├── .gitignore
└── README.md


📌 Observação:
As pastas raylib/ e mingw-w64/ NÃO vão para o GitHub.
São ferramentas do desenvolvedor e não fazem parte do projeto.

🔧 Como Compilar (Windows)
📦 Dependências necessárias

Raylib 5.0 (Win64)

GCC WinLibs (mingw-w64)

SQLite3 (já incluso)

curl.exe instalado em:
C:\curl\curl.exe

▶️ Compilação

Dentro da pasta /src, execute:

build.bat


O script gera:

PetControl.exe

🧩 2) PetControl Web — Plataforma Online

Sistema moderno e completo para gestão de pet shops, desenvolvido em Java + MySQL + HTML/CSS/JS.

✨ Funcionalidades Principais

📅 Agenda inteligente

💰 PDV com emissão de notas

🐶 Cadastro completo dos pets

👥 Gestão de clientes

📦 Controle de estoque com alertas

📊 Relatórios de desempenho

🌐 Interface moderna e responsiva

🌐 Demonstração Online

Acesse:

👉 https://luanasacutti.github.io/PetControl/

Ou abra localmente:

open index.html

👩‍💻 Equipe Desenvolvedora
Nome	Função
Guilherme Almeida	Back-end & Database
Luana Sacutti	Full-stack Development
Maria Eduarda Ferraz	Front-end & Design
📞 Contato

Desenvolvido com ❤️ pela equipe PetControl.
