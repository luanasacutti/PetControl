
const API_BASE = '';

// Estado global da aplicação
const AppState = {
    clientes: [],
    agendamentos: [],
    vendas: [],
    compras: [],
    estoque: []
};

// Inicialização da aplicação
document.addEventListener('DOMContentLoaded', function() {
    console.log('🚀 PetControl Frontend Iniciado');
    inicializarAplicacao();
    configurarFormularios();

    const vencInput = document.getElementById('vencimentoCliente');
    if (vencInput) vencInput.valueAsDate = new Date();
});

// Inicializar aplicação
async function inicializarAplicacao() {
    try {
        await verificarBackend();
        await carregarClientes();
        await carregarVencimentos();
        console.log('✅ Aplicação inicializada com sucesso');
    } catch (error) {
        console.error('❌ Erro na inicialização:', error);
        mostrarMensagem('Erro ao conectar com o servidor', 'error');
    }
}

// Verificar se o backend está funcionando
async function verificarBackend() {
    try {
        const response = await fetch(`${API_BASE}/health`);
        const data = await response.json();
        console.log('✅ Backend conectado:', data);
        return true;
    } catch (error) {
        console.error('❌ Erro ao conectar com backend:', error);
        mostrarMensagem('⚠️ Backend offline - Algumas funcionalidades podem não funcionar', 'warning');
        return false;
    }
}

// Configurar todos os formulários
function configurarFormularios() {
    const clienteForm = document.getElementById('clientePlanoForm');
    if (clienteForm) {
        clienteForm.addEventListener('submit', function(e) {
            e.preventDefault();
            cadastrarCliente();
        });
    }

    const contactForm = document.getElementById('contactForm');
    if (contactForm) {
        contactForm.addEventListener('submit', function(e) {
            e.preventDefault();
            enviarContato();
        });
    }
}

// Cadastrar cliente no backend
async function cadastrarCliente() {
    const btnSubmit = document.querySelector('#clientePlanoForm button[type="submit"]');
    const btnOriginalText = btnSubmit ? btnSubmit.innerHTML : 'Salvar';

    if (btnSubmit) {
        btnSubmit.innerHTML = '<i class="fas fa-spinner fa-spin me-2"></i>Cadastrando...';
        btnSubmit.disabled = true;
    }

    const cliente = {
        nome: (document.getElementById('nomeCliente') || {}).value || '',
        email: (document.getElementById('emailCliente') || {}).value || '',
        telefone: (document.getElementById('telefoneCliente') || {}).value || '',
        cpf_cnpj: (document.getElementById('cpfCliente') || {}).value || '',
        plano: (document.getElementById('planoCliente') || {}).value || '',
        vencimento: (document.getElementById('vencimentoCliente') || {}).value || null
    };

    try {
        const response = await fetch(`${API_BASE}/clientes`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(cliente)
        });

        const result = await response.json();

        if (response.ok) {
            mostrarMensagem('✅ Cliente cadastrado com sucesso no banco de dados!', 'success');
            const formEl = document.getElementById('clientePlanoForm');
            if (formEl) formEl.reset();
            await carregarClientes();
            await carregarVencimentos();
        } else {
            throw new Error(result.error || 'Erro ao cadastrar cliente');
        }
    } catch (error) {
        console.error('❌ Erro:', error);
        mostrarMensagem('❌ Erro ao cadastrar cliente: ' + error.message, 'error');
    } finally {
        if (btnSubmit) {
            btnSubmit.innerHTML = btnOriginalText;
            btnSubmit.disabled = false;
        }
    }
}

// Carregar clientes do backend
async function carregarClientes() {
    try {
        const tbody = document.getElementById('clientesList');
        if (!tbody) return;

        tbody.innerHTML = `
            <tr>
                <td colspan="7" class="text-center text-muted">
                    <i class="fas fa-spinner fa-spin me-2"></i>Carregando clientes...
                </td>
            </tr>
        `;

        const response = await fetch(`${API_BASE}/clientes`);
        if (!response.ok) throw new Error('Erro ao carregar clientes');

        const clientes = await response.json();
        AppState.clientes = clientes;

        if (clientes.length === 0) {
            tbody.innerHTML = `
                <tr>
                    <td colspan="7" class="text-center text-muted">
                        <i class="fas fa-users me-2"></i>Nenhum cliente cadastrado
                    </td>
                </tr>
            `;
            return;
        }

        tbody.innerHTML = '';

        clientes.forEach(cliente => {
            const dataVencimento = cliente.vencimento ? new Date(cliente.vencimento) : null;
            const hoje = new Date();
            const diffTime = dataVencimento ? (dataVencimento - hoje) : Infinity;
            const diffDays = dataVencimento ? Math.ceil(diffTime / (1000 * 60 * 60 * 24)) : '-';

            let vencimentoClass = 'vencimento-normal';
            let statusText = '';

            if (dataVencimento) {
                if (diffDays <= 0) {
                    vencimentoClass = 'vencimento-hoje';
                    statusText = ' (Hoje!)';
                } else if (diffDays <= 7) {
                    vencimentoClass = 'vencimento-proximo';
                    statusText = ` (em ${diffDays} dias)`;
                }
            }

            const tr = document.createElement('tr');
            tr.className = vencimentoClass;
            tr.innerHTML = `
                <td><strong>${cliente.id}</strong></td>
                <td>${cliente.nome}</td>
                <td>${cliente.email || '-'}</td>
                <td>${cliente.telefone || '-'}</td>
                <td><span class="badge bg-primary">${cliente.plano || '-'}</span></td>
                <td>${dataVencimento ? dataVencimento.toLocaleDateString('pt-BR') + statusText : '-'}</td>
                <td>
                    <button class="btn btn-sm btn-outline-warning me-1" onclick="editarCliente(${cliente.id})" title="Editar">
                        <i class="fas fa-edit"></i>
                    </button>
                    <button class="btn btn-sm btn-outline-info me-1" onclick="enviarLembrete(${cliente.id})" title="Enviar Lembrete">
                        <i class="fas fa-envelope"></i>
                    </button>
                    <button class="btn btn-sm btn-outline-danger" onclick="excluirCliente(${cliente.id})" title="Excluir">
                        <i class="fas fa-trash"></i>
                    </button>
                </td>
            `;
            tbody.appendChild(tr);
        });
    } catch (error) {
        console.error('❌ Erro ao carregar clientes:', error);
        const tbody = document.getElementById('clientesList');
        if (tbody) {
            tbody.innerHTML = `
                <tr>
                    <td colspan="7" class="text-center text-danger">
                        <i class="fas fa-exclamation-triangle me-2"></i>Erro ao carregar clientes: ${error.message}
                    </td>
                </tr>
            `;
        }
    }
}

// Carregar vencimentos do backend
async function carregarVencimentos() {
    try {
        const tbody = document.getElementById('vencimentosList');
        if (!tbody) return;

        const diasInput = document.getElementById('diasVencimento');
        const dias = diasInput && diasInput.value ? diasInput.value : 7;

        tbody.innerHTML = `
            <tr>
                <td colspan="4" class="text-center text-muted">
                    <i class="fas fa-spinner fa-spin me-2"></i>Carregando vencimentos...
                </td>
            </tr>
        `;

        const response = await fetch(`${API_BASE}/vencimentos?days=${encodeURIComponent(dias)}`);
        if (!response.ok) throw new Error('Erro ao carregar vencimentos');

        const vencimentos = await response.json();

        if (vencimentos.length === 0) {
            tbody.innerHTML = `
                <tr>
                    <td colspan="4" class="text-center text-muted">
                        <i class="fas fa-check me-2"></i>Nenhum vencimento próximo
                    </td>
                </tr>
            `;
            return;
        }

        tbody.innerHTML = '';

        vencimentos.forEach(cliente => {
            const dataVencimento = cliente.vencimento ? new Date(cliente.vencimento) : null;
            const hoje = new Date();
            const diffTime = dataVencimento ? (dataVencimento - hoje) : Infinity;
            const diffDays = dataVencimento ? Math.ceil(diffTime / (1000 * 60 * 60 * 24)) : '-';

            let badgeClass = 'bg-success';
            let statusText = `${diffDays} dias`;

            if (dataVencimento) {
                if (diffDays <= 0) {
                    badgeClass = 'bg-danger';
                    statusText = 'Hoje!';
                } else if (diffDays <= 3) {
                    badgeClass = 'bg-danger';
                } else if (diffDays <= 7) {
                    badgeClass = 'bg-warning';
                }
            } else {
                statusText = '-';
            }

            const tr = document.createElement('tr');
            tr.innerHTML = `
                <td>${cliente.nome}</td>
                <td><span class="badge bg-primary">${cliente.plano || '-'}</span></td>
                <td>
                    <span class="badge ${badgeClass}">
                        ${dataVencimento ? dataVencimento.toLocaleDateString('pt-BR') + ` (${statusText})` : '-'}
                    </span>
                </td>
                <td>
                    <button class="btn btn-sm btn-outline-info" onclick="enviarLembrete(${cliente.id})">
                        <i class="fas fa-envelope me-1"></i>Lembrete
                    </button>
                </td>
            `;
            tbody.appendChild(tr);
        });
    } catch (error) {
        console.error('❌ Erro ao carregar vencimentos:', error);
        const tbody = document.getElementById('vencimentosList');
        if (tbody) {
            tbody.innerHTML = `
                <tr>
                    <td colspan="4" class="text-center text-danger">
                        <i class="fas fa-exclamation-triangle me-2"></i>Erro ao carregar vencimentos
                    </td>
                </tr>
            `;
        }
    }
}

// Enviar lembrete por email
async function enviarLembrete(clienteId) {
    try {
        const cliente = AppState.clientes.find(c => c.id === clienteId);
        if (!cliente) throw new Error('Cliente não encontrado');

        const response = await fetch(`${API_BASE}/enviar-email`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                email: cliente.email,
                nome: cliente.nome,
                vencimento: cliente.vencimento
            })
        });

        const result = await response.json();

        if (response.ok) {
            mostrarMensagem(`✅ Lembrete enviado com sucesso para ${cliente.email}`, 'success');
        } else {
            throw new Error(result.error || 'Erro ao enviar email');
        }
    } catch (error) {
        console.error('❌ Erro ao enviar lembrete:', error);
        mostrarMensagem('❌ Erro ao enviar lembrete: ' + error.message, 'error');
    }
}

// Excluir cliente
async function excluirCliente(clienteId) {
    if (!confirm('Tem certeza que deseja excluir este cliente?')) return;

    try {
        const response = await fetch(`${API_BASE}/clientes/${clienteId}`, { method: 'DELETE' });

        if (response.ok) {
            mostrarMensagem('✅ Cliente excluído com sucesso!', 'success');
            await carregarClientes();
            await carregarVencimentos();
        } else {
            const result = await response.json();
            throw new Error(result.error || 'Erro ao excluir cliente');
        }
    } catch (error) {
        console.error('❌ Erro ao excluir cliente:', error);
        mostrarMensagem('❌ Erro ao excluir cliente: ' + error.message, 'error');
    }
}

// Executar operação C
async function executarOperacaoC() {
    try {
        const response = await fetch(`${API_BASE}/c-operation`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ operation: 'buscar_clientes', data: { limit: 10 } })
        });

        const result = await response.json();

        if (response.ok) {
            mostrarMensagem('✅ Operação C executada com sucesso!', 'success');
            console.log('🔧 Resultado da operação C:', result);
        } else {
            throw new Error(result.error || 'Erro na operação C');
        }
    } catch (error) {
        console.error('❌ Erro na operação C:', error);
        mostrarMensagem('❌ Erro na operação C: ' + error.message, 'error');
    }
}

// Função para editar cliente
function editarCliente(clienteId) {
    const cliente = AppState.clientes.find(c => c.id === clienteId);
    if (!cliente) return;

    const setIf = (id, value) => { const el = document.getElementById(id); if (el) el.value = value || ''; };

    setIf('nomeCliente', cliente.nome);
    setIf('emailCliente', cliente.email || '');
    setIf('telefoneCliente', cliente.telefone || '');
    setIf('cpfCliente', cliente.cpf_cnpj || '');
    setIf('planoCliente', cliente.plano || '');
    setIf('vencimentoCliente', cliente.vencimento || '');

    const gestaoEl = document.getElementById('gestao');
    if (gestaoEl) gestaoEl.scrollIntoView({ behavior: 'smooth' });

    mostrarMensagem(`✏️ Editando cliente: ${cliente.nome} - Atualize os dados e clique em "Cadastrar"`, 'info');
}

// Enviar formulário de contato
async function enviarContato() {
    mostrarMensagem('✅ Mensagem enviada com sucesso! Entraremos em contato em breve.', 'success');
    const formEl = document.getElementById('contactForm');
    if (formEl) formEl.reset();
}

// Funções auxiliares
function mostrarMensagem(mensagem, tipo) {
    const alert = document.createElement('div');
    alert.className = `alert alert-${tipo === 'error' ? 'danger' : tipo === 'warning' ? 'warning' : 'success'} alert-dismissible fade show position-fixed`;
    alert.style.top = '20px';
    alert.style.right = '20px';
    alert.style.zIndex = '9999';
    alert.style.minWidth = '300px';
    alert.style.maxWidth = '400px';
    alert.innerHTML = `
        <div class="d-flex align-items-center">
            <i class="fas fa-${tipo === 'success' ? 'check' : tipo === 'warning' ? 'exclamation-triangle' : 'exclamation-circle'} me-2"></i>
            <div>${mensagem}</div>
        </div>
        <button type="button" class="btn-close" data-bs-dismiss="alert"></button>
    `;
    document.body.appendChild(alert);
    setTimeout(() => { if (alert.parentNode) alert.parentNode.removeChild(alert); }, 5000);
}

function scrollToDemo() {
    const el = document.getElementById('demo');
    if (el) el.scrollIntoView({ behavior: 'smooth' });
}

function mostrarModal(titulo, mensagem) {
    const titleEl = document.getElementById('modalTitle');
    const msgEl = document.getElementById('modalMessage');
    const modalEl = document.getElementById('confirmModal');
    if (titleEl) titleEl.textContent = titulo;
    if (msgEl) msgEl.textContent = mensagem;
    if (modalEl) modalEl.style.display = 'flex';
}

function fecharModal() {
    const modalEl = document.getElementById('confirmModal');
    if (modalEl) modalEl.style.display = 'none';
}