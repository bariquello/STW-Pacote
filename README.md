# STW-Pacote — pfSense (System Way)

Pacote interno da **System Way** para **pfSense CE 2.8 / pfSense Plus 24.x** que automatiza duas tarefas hoje feitas manualmente em toda implantação de firewall:

1. **Aplica o tema `pfSense-Systemway`** — CSS, JS, `head.inc`, logo e favicons da System Way, incluindo a tela de login customizada e o branding *"Systemway Firewall"* no menu.
2. **Cria o menu `Services → STW Backup FTP`** — um formulário que gera o `/etc/backup.sh` e, opcionalmente, agenda o backup diário no cron. Substitui o procedimento manual da **Base de Conhecimento 338**.

**Versão atual: 0.5**

---

## 📑 Índice

- [Como funciona](#-como-funciona)
- [Estrutura do repositório](#-estrutura-do-repositório)
- [Instalação em um firewall](#-instalação-em-um-firewall)
- [Instalação pelo Command Prompt (web)](#-instalação-pelo-command-prompt-web)
- [Atualizar um firewall já instalado](#-atualizar-um-firewall-já-instalado)
- [Deploy: como publicar alterações no GitHub](#-deploy-como-publicar-alterações-no-github)
- [Usando o menu STW Backup FTP](#-usando-o-menu-stw-backup-ftp)
- [Desinstalação](#-desinstalação)
- [Troubleshooting](#-troubleshooting)
- [Histórico de versões](#-histórico-de-versões)

---

## 🔧 Como funciona

O pacote **não precisa ser compilado**. Ele é composto apenas de PHP e arquivos estáticos, então a instalação consiste em colocar os arquivos nos caminhos corretos e registrar o pacote no `config.xml`.

### Fluxo da instalação

```
[ Firewall pfSense ]                         [ GitHub: bariquello/STW-Pacote ]
        |                                                    |
        |  fetch stw_setup.sh | sh  ───────────────────────>  |
        |                                                    |
        |  <────────────── stw-pacote-files.tar.gz ────────── |
        |
        ├─ tar -xzf ... -C /        → arquivos em /usr/local/...
        ├─ chmod +x rc.d/stw_backup.sh
        ├─ registra STW-Pacote em installedpackages/package
        └─ stw_pacote_install()
              ├─ stw_deploy_theme()     → copia tema para /usr/local/www
              ├─ stw_aplicar_tema()     → webguicss = pfSense-Systemway.css
              ├─ stw_registrar_menu()   → Services > STW Backup FTP
              └─ stw_pacote_resync()    → gera /etc/backup.sh
```

### Componentes

| Arquivo | Função |
|---|---|
| `stw_pacote.xml` | Define a GUI (campos), o menu em *Services*, o serviço e os hooks de ciclo de vida |
| `stw_pacote.inc` | Lógica: deploy do tema, registro do menu, geração do `backup.sh` e agendamento do cron |
| `info.xml` | Manifesto do pacote |
| `theme/` | Assets reais do tema (CSS, JS, `head.inc`, logo, favicons) |
| `stw_backup.sh` | Script `rc.d` para executar o backup sob demanda |

### Hooks de ciclo de vida

| Hook (no XML) | Função (no `.inc`) | Quando roda |
|---|---|---|
| `custom_php_install_command` | `stw_pacote_install()` | Na instalação |
| `custom_php_resync_config_command` | `stw_pacote_resync()` | **Ao salvar o formulário** |
| `custom_php_deinstall_command` | `stw_pacote_deinstall()` | Na remoção |

### Tratamento do tema

Os arquivos **core** do pfSense (`head.inc`, `pfSense.js`, `pfSenseHelpers.js`, `polyfills.js`, `traffic-graphs.js`) são **respaldados** em `/usr/local/share/pfSense-pkg-STW-Pacote/orig_backup/` antes de serem sobrescritos. Na desinstalação eles são restaurados e o tema anterior volta a valer.

> Após um **update do pfSense** (que sobrescreve arquivos core), reaplique o tema marcando a opção *"Reaplicar tema pfSense-Systemway"* no formulário e salvando.

---

## 📂 Estrutura do repositório

```
STW-Pacote/                                  ← raiz do repositório
├── stw_setup.sh                             ← instalador (one-liner)
├── stw_update.sh                            ← atualizador
├── build_dist.sh                            ← regera o tarball
├── stw-pacote-files.tar.gz                  ← pacote distribuído (árvore relativa a /)
├── README.md
├── .gitignore
└── files/usr/local/
    ├── pkg/
    │   ├── stw_pacote.xml                   ← GUI + menu + hooks
    │   └── stw_pacote.inc                   ← lógica do pacote
    ├── share/pfSense-pkg-STW-Pacote/
    │   ├── info.xml
    │   └── theme/
    │       ├── css/   pfSense-Systemway.css · pfSense-dark.css · login.css
    │       ├── js/    pfSense.js · pfSenseHelpers.js · polyfills.js · traffic-graphs.js
    │       └── www/   head.inc · logo.png · logo-branco.png
    │                  favicon.ico · favicon-16x16.png · favicon-32x32.png
    └── etc/rc.d/stw_backup.sh
```

> ⚠️ A pasta `files/` espelha exatamente a árvore de destino no firewall. O tarball é gerado a partir do **conteúdo** dela (começando em `usr/`), para que `tar -xzf ... -C /` extraia direto nos caminhos corretos.

---

## 🚀 Instalação em um firewall

Acesse o firewall por **SSH (opção 8 — Shell)**. Como o shell padrão do root é o **tcsh**, entre primeiro no `sh` para evitar erros de interpretação:

```sh
sh
fetch -q -o - "https://raw.githubusercontent.com/bariquello/STW-Pacote/main/stw_setup.sh" | sh
```

Ao final:

1. Recarregue a interface web com **Ctrl+F5** (ou faça logout/login).
2. Acesse **Services → STW Backup FTP** e configure o backup.

---

## 🌐 Instalação pelo Command Prompt (web)

O **Diagnostics → Command Prompt** executa comandos de forma **não interativa**: ele roda o comando, aguarda o término e só então exibe a saída. Não há shell interativo e o ambiente (`PATH`, variáveis) é mais restrito que o do SSH — por isso o `fetch ... | sh` direto pode não funcionar como no terminal.

**Use dois comandos separados, com caminhos absolutos:**

**1º comando** — baixar o script:

```sh
/usr/bin/fetch -o /tmp/stw_setup.sh https://raw.githubusercontent.com/bariquello/STW-Pacote/main/stw_setup.sh
```

**2º comando** — executar:

```sh
/bin/sh /tmp/stw_setup.sh 2>&1
```

Observações:

- O `2>&1` é importante para que mensagens de erro apareçam na tela do navegador.
- A saída só é exibida **depois** que o script termina — aguarde alguns instantes sem recarregar a página.
- Se a página expirar antes de concluir, prefira executar via **SSH**, que é o método recomendado.

---

## 🔄 Atualizar um firewall já instalado

```sh
sh
fetch -q -o - "https://raw.githubusercontent.com/bariquello/STW-Pacote/main/stw_update.sh" | sh
```

O `stw_update.sh` baixa a versão nova, reaplica o tema, garante o menu, regenera o `/etc/backup.sh` a partir da configuração salva e exibe as linhas `HOST=`, `USUARIO=` e `SENHA=` para conferência.

---

## 📦 Deploy: como publicar alterações no GitHub

Sempre que alterar o tema ou a lógica do pacote, siga estes passos.

### 1. Edite os arquivos dentro de `files/`

| O que mudar | Onde editar |
|---|---|
| Cores, layout, login | `files/usr/local/share/pfSense-pkg-STW-Pacote/theme/css/` |
| Logo, favicons | `files/usr/local/share/pfSense-pkg-STW-Pacote/theme/www/` |
| Menu, branding do topo | `files/usr/local/share/pfSense-pkg-STW-Pacote/theme/www/head.inc` |
| Campos do formulário | `files/usr/local/pkg/stw_pacote.xml` |
| Lógica / script de backup | `files/usr/local/pkg/stw_pacote.inc` |

### 2. Regere o tarball

O tarball **precisa** ser regerado após qualquer alteração em `files/`, pois é ele que os firewalls baixam.

**Windows (PowerShell, na raiz do repo):**

```powershell
tar -czf stw-pacote-files.tar.gz -C files usr
```

**Linux / macOS / Git Bash / WSL:**

```sh
sh build_dist.sh
```

**Confira a estrutura interna** antes de commitar:

```powershell
tar -tzf stw-pacote-files.tar.gz
```

Deve começar com `usr/local/...` — **nunca** com `files/usr/...`.

### 3. Commit e push

```sh
git add .
git commit -m "Atualiza tema e regenera tarball"
git push
```

### 4. Aplique nos firewalls

Rode o `stw_update.sh` em cada firewall (seção [Atualizar](#-atualizar-um-firewall-já-instalado)).

> 💡 Como os scripts leem do branch `main` via `raw.githubusercontent.com`, a alteração fica disponível imediatamente após o push. Não é necessário criar release nem tag.

---

## ⚙️ Usando o menu STW Backup FTP

Acesse **Services → STW Backup FTP**.

| Campo | Descrição | Padrão |
|---|---|---|
| **Agendar backup diário (cron)** | Cria a tarefa no cron. O `backup.sh` é gerado mesmo sem marcar. | desmarcado |
| **Servidor FTP (HOST)** | Servidor de destino do backup | `monitoramento.systemway.com.br` |
| **Porta FTP** | Porta do serviço | `21000` |
| **Usuário FTP** | Altere conforme a empresa/cliente | `pfsense` |
| **Senha FTP** | Senha do usuário FTP | — |
| **Prefixo do arquivo (ARQUIVO)** | Padrão dos arquivos enviados — **atente-se ao nome do firewall** | `firewall.*` |
| **Horário do backup diário** | Hora (0–23) para o cron; minuto fixo em `0` | `3` |
| **Reaplicar tema pfSense-Systemway** | Refaz o deploy do tema em `/usr/local/www` | desmarcado |

Ao clicar em **Save**, o pfSense chama o `stw_pacote_resync()`, que **sempre** regenera o `/etc/backup.sh` com os valores do formulário.

### Validação

```sh
cat /etc/backup.sh                          # confere o script gerado
grep -E "^HOST=|^USUARIO=|^SENHA=" /etc/backup.sh
sh /etc/backup.sh                           # execução manual de teste
crontab -l | grep backup.sh                 # confere o agendamento
```

No servidor FTP devem chegar: `NOME-DD_MM_AAAA.xml`, `Relatorio_Diario_Backup.txt` e `Relatorio_Mensal_Backup.txt`.

---

## 🗑️ Desinstalação

```sh
sh
php -q -r 'require_once("config.inc");require_once("functions.inc");require_once("pkg-utils.inc");require_once("/usr/local/pkg/stw_pacote.inc");stw_pacote_deinstall();echo "removido\n";'
```

A desinstalação restaura os arquivos core originais, remove os CSS/logos do tema, retorna o `webguicss` anterior, remove o menu, apaga o `/etc/backup.sh` e remove a tarefa do cron. Os relatórios em `/cf/conf/` são preservados.

---

## 🧰 Troubleshooting

| Sintoma | Causa provável | Solução |
|---|---|---|
| `git: Command not found` | O pfSense não inclui `git` nem compilador — por design | Não é necessário compilar; use `fetch` + `tar` |
| `Host does not resolve` | URL placeholder ou branch incorreto no script | Confira `RAW=` no topo do `stw_setup.sh` |
| `Badly placed ()'s` / prompts `?` | Comando colado no **tcsh** (shell padrão do root) | Digite `sh` antes de colar |
| Tema aplicou, mas **sem logo** | Binários ausentes no tarball | Verifique os 5 arquivos em `theme/www/` e regere o tarball |
| Menu não aparece em *Services* | Menu não registrado ou cache de sessão | Rode o `stw_update.sh` e faça **logout/login** |
| Senha não atualiza no `backup.sh` | Configuração lida de chave divergente (corrigido na v0.5) | Atualize para a v0.5 e salve o formulário uma vez |
| Command Prompt (web) sem saída | Execução não interativa / tempo de execução | Use os dois comandos com caminho absoluto ou prefira SSH |

### Conferir o que a GUI salvou

```sh
sh
php -q -r 'require_once("config.inc");var_dump(config_get_path("installedpackages/stwpacote/config/0",array()));'
```

Se retornar `array(0) {}`, o formulário ainda não foi salvo nesse firewall — abra **Services → STW Backup FTP** e clique em **Save**.

### Logs

```sh
clog /var/log/system.log | grep STW-Pacote
```

---

## 📌 Histórico de versões

| Versão | Alterações |
|---|---|
| **0.5** | Corrige a leitura da configuração da GUI (`<name>` alinhado ao `<configpath>`), adiciona busca tolerante da config, `html_entity_decode()` e escape de aspas nos valores |
| **0.4** | `backup.sh` passa a ser **sempre** gerado ao salvar (o checkbox controla apenas o cron); registro do menu embutido no install/resync |
| **0.3** | Deploy do tema com backup/restauração dos arquivos core; instalador `fetch \| sh` |

---

*System Way — uso interno.*
