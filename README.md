# P2P_TrackerAndClient
 Software for data sharing (similar to torrent) using Win32 and C programming language . Only works in Windows OS..
 
-------------------------------------------
Realizado por:
	André Vitor Beraldo 
	Felipe dos Santos 

-------------------------------------------

	Foram desenvolvidos dois programas, um cliente e um rastreador, para a transferência de video ou 
música entre sistemas finais diferentes.
	A implementação executada conta com um banco de dados
básico para que o rastreador mantenha as informações dos usuá-
rios com seus respectivos arquivos.

	Para executarmos o código corretamente devemos:
	
	- Definir os clientes participantes, cada um em sua 
pasta, com uma versão CLIENTE à disposição. Dentro de cada
pasta de clientes pode-se ter músicas ou vídeos para compar-
tilhamento;
	- Definir uma pasta para o Rastreador;
	- Abrir todos os programas. Os programas CLIENTEs
perguntarão o IP do rastreador, que deve ser informado para
a comunicação entre as aplicações.
	- Por Default, o banco de dados vem desatualizado e
cabe ao usuário atualizado. Para isso, o rastreador deve entrar 
no modo "3- Habilitar atualização de BD". Assim, todos os 
usuários podem adicionar seus arquivos com a função " 4- Atualizar
Banco de Dados". (Após a inserção, o Rastreador estará em loop
infinito, basta reiniciá-lo). Agora o banco de dados está atualizado
com os arquivos de cada usuário com seu respectivo endereço IP atual.
	- Agora sabemos qual usuário é detentor de cada arquivo.
	- Para um cliente requisitar um arquivo, é necesário que o 
Rastreador esteja com o Servidor ativo através da função 1. O servidor
tem a função de fazer a ponte entre o requisitante e o detentor do
arquivo pesquisado.
	- Para que haja a transferência, o detentor do arquivo deve
estar no estado de "Semeando", podendo enviar o arquivo.
	- Caso o arquivo esteja correto, a transferência será executada!
	
