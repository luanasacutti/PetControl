📘 PetControl – Gerenciamento de Planos Comprados no Site

O PetControl é um sistema desenvolvido em C (Raylib + SQLite) para facilitar o controle interno dos planos que os clientes compram pelo site.
Ele automatiza consultas, relatórios e notificações, ajudando no gerenciamento diário de forma simples e eficiente.

🐾 ✨ Funcionalidades principais

🔍 Consulta de cadastros feitos pelo site
Busca rápida de clientes e informações do plano adquirido.

📊 Geração de relatórios
Exportação de dados em formatos como CSV, permitindo análises e controle.

✉️ Envio automático de e-mails
O sistema envia alertas quando um plano está próximo do vencimento, garantindo melhor acompanhamento dos clientes.

🗂️ 📁 Estrutura do projeto
PetControl/
 ├── src/
 │   ├── main.c
 │   ├── painel.c
 │   ├── shell.c
 │   ├── sqlite3.c
 │   ├── sqlite3.h
 │   ├── build.bat
 │
 ├── assets/
 │   └── logo.png
 │
 ├── .gitignore
 ├── README.md

🔧 🛠️ Como compilar

O projeto utiliza:

Raylib 5.0 (Win64)

GCC WinLibs (mingw-w64)

SQLite3

Para compilar:

./src/build.bat


O script usa os caminhos configurados para Raylib e GCC e gera:

PetControl.exe

🗄️ 💾 Banco de Dados

Utiliza SQLite3

Os arquivos .db são locais e não são enviados para o GitHub

O sistema cria ou manipula os bancos automaticamente

🎯 Objetivo do Sistema

Facilitar o gerenciamento dos planos comprados pelo site, oferecendo:

Organização centralizada

Relatórios internos

Alertas automáticos

Consulta rápida dos clientes

Mais eficiência operacional

👩‍💻 Desenvolvedora

Luana Sacutti