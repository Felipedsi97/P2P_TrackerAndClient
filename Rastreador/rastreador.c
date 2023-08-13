              /*
              Universidade Federal de Itajubá.
              COM240 - Redes de computadores.
              Professor Bruno Guazelli Batista.
              Programa para enviar arquivos via socket entre sistemas finais diferentes.
              Protocolo UDP.
              Implementado em C para windows.
              Contém soma de verificação, número de sequência, temporizador e confirmação de recebimento.
              Versão: RASTREADOR.
              Realizado por:
              André Vitor Beraldo - 34463.
              Felipe dos Santos - 2017013090.
              Data:08/06/18.
              */




#ifdef _WIN32          // Comando para identificar o SO e negar execução em sistemas diferentes do Windows.
#include <winsock2.h>  //Biblioteca voltada para a utilização de sockets.
#include <windows.h>   //Biblioteca para utilizar comandos para Windows, inclusive o Winsock, sockets no Windows.
#include <stdio.h>     //Biblioteca padrão para comandos básicos.
#include <stdlib.h>    //Biblioteca para alguns comandos do sistema.
#include <string.h>    //Biblioteca para manipulação de cadeia de caracteres.
#include <conio.h>     //Biblioteca para ler ou manipular cadeia de caracteres.
#include <locale.h>    //Biblioteca para deixar o programa na linguagem do sistema.

#else

// Dá mensagem de erro informando que o sistemas não é suportado.
#error "Sistema nao suportado."

#endif

#pragma comment(lib,"ws2_32.lib")   //comando para utilizar a winsock.dll.
#define BUFLEN 512                  //Tamanho máximo do buffer.
#define PORT 8888                   //Porta pela qual serão passados os dados.

#define IP_RAST                     //IP utilizado para hospedar o rastreador.

// PROTÓTIPOS
int menu();     											//Função para o menu principal do programa.
void refresh();    											//Função para atualizar a frontend do programa.
void returnClick(); 										//Função para retornar o programa para a página anterior.
int servidorON();   										//Função utilizada para iniciar o socket na rede.
int verificarBancoDados(char *requisicaoBuf);  				//Função utilizada para verificar o conteúdo presente no banco de dados.
int responderCliente(char *mensagem, char *IP_CLIENTE);  	//Função utilizada para enviar uma resposta ao cliente.
void aguardarArquivos();  									//Função utilizada para aguardar a resposta dos clientes.
int exibirBancoDeDados();   								//Função utilizada para mostrar os arquivos presentes no banco de dados.
int main(); 												//Função principal.

// FUNÇÕES

int menu()
{
    int opcao=0;
    while(1)
    {
        refresh();                                                  //Inicia o menu do programa, com opções para o usuário escolher.
        printf("\nOPCOES:\n");
        printf("   1- Ativar o servidor\n");
        printf("   2- Exibir banco de dados\n");
        printf("   3- Habilitar atualização de BD\n");
        printf("   0- Fechar rede\n\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);
        switch(opcao)
        {
        case 1 :                                                //Atualiza a interface, inicia o rastreador na rede e retorna a página inicial.
            refresh();
            servidorON();
            returnClick();
            break;
        case 2 :                                                //Atualiza a interfacem, exibe o conteúdo presente no banco de dados e retorna a página inicial.
            refresh();
            exibirBancoDeDados();
            returnClick();
            break;
        case 3 :                                                //Atualiza a interface, atualiza o banco de dados e retorna a página inicial.
            refresh();
            aguardarArquivos();
            returnClick();
            break;
        case 0 :                                                //Sai do programa.
            refresh();
            printf("SISTEMA CONCLUIDO");
            exit(0);
            break;
        default :                                               //Retorna a pagina inicial após qualquer comando não presente nos casos.
            refresh();
            printf ("Valor invalido!\n");
            returnClick();
        }
    }
    return;
}

void refresh()
{
    system("cls");
    printf("-----------------------------------------\n");
    printf("|\tTRANSFERENCIA DE ARQUIVOS MP3\t|\n");
    printf("-----------------------------------------\n");                      //Design do cabeçalho do programa.
    printf("\t\tRASTREADOR \t\t\n");
    printf("-----------------------------------------\n");
}

void returnClick()
{
    printf("\nAperte um botao para retornar...");                               //Função para retornar.
    getch();                                                                    //Utilizado para não deixar o programa mostrar a mensagem e retirá-la sem um comando.
    refresh();
}

int servidorON()
{
    SOCKET s;                                                                   //Função para iniciar o socket.
    struct sockaddr_in server, si_other;                                        //Structs com definições da conexão.
    int slen, recv_len;
    char buf[BUFLEN];                                                           //Cadeia de caracter.
    char mensagem[BUFLEN];                                                       //Variável para utilizar a winsock2.dll.
    WSADATA wsa;                                                                //Ponteiro auxiliar.
    char *alvo;                                                                 //Verificar tamanho da struct.

    slen = sizeof(si_other) ;


    printf("\nInicializando Winsock...");                                       //Inicialização do Winsock.
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
        printf("Erro de Winsock\n");
        returnClick();
    }
    printf("\n\tInicializado.\n");

    if((s = socket(AF_INET, SOCK_DGRAM, 0 )) == INVALID_SOCKET)                 //Cria o socket.
    {
        printf("Socket nao pode ser criado\n");
    }
    printf("\tSocket criado.\n");

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;                                        //Prepara a estrutura com as informações da conexão e do socket.
    server.sin_port = htons( PORT );


    if( bind(s,(struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)     //Bind: Liga a porta ao socket na rede.
    {
        printf("Erro de bind()\n");
        returnClick();
    }
    puts("\tBind realizado.");
    printf("Aguardando transacoes...\n");

    while(1)                                                                    //Loop para recebimento de requisição.
    {
        fflush(stdout);
        memset(buf,'\0', BUFLEN);                                               //Limpa o buffer para recebimento de dados.

                                                                                //Prepara o programa para entrar em modo de recebimento.
        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
        {
            printf("Erro de recvfrom()!\n");
            returnClick();
        }
        char *IP_CLIENTE = inet_ntoa(si_other.sin_addr);
        printf("\nPacote recebido de %s", inet_ntoa(si_other.sin_addr));        //Exibe as requisições dos clientes com seu ip e porta.
        printf("\nRequisicao: %s\n", buf);


        alvo=verificarBancoDados(buf);                                          //Flag para determinar se a requisição foi encontrada.
        if(alvo == 0)
        {
            printf("Arquivo nao encontrado.\n");
            alvo="false";
            responderCliente(alvo,IP_CLIENTE);
        }
        else
        {
            responderCliente(alvo,IP_CLIENTE);
        }

    }

    close(s);                                                                   //Fecha o socket.
    WSACleanup();                                                               //Termina o uso da winsock2.dll.

    return 0;

}

int verificarBancoDados(char *requisicaoBuf)
{
    char ip[16], chave[32];
    int i;
    FILE *arq = fopen("bancoDeDados.txt", "rt");                                //Abre o arquivo bando de dados.txt para encontrar os arquivos disponíveis.
    while( (fscanf(arq, "%s %s\n", &ip, &chave)) != EOF)                        //Percorre o arquivo para verificar se a requisição existe no BD.
    {
        if((strcmp(chave, requisicaoBuf))== 0)
        {
            printf("O arquivo '%s' foi encontrado no cliente %s\n", &chave, &ip);
            fclose(arq);
            return ip;
        }
    }
    fclose(arq);                                                                //Fecha o arquivo.txt.
    return 0;
}

int responderCliente(char *mensagem, char *IP_CLIENTE)                           //Função para enviar resposta ao cliente.
{
    char *ip = (char*) malloc(strlen(mensagem)*sizeof(char));
    strcpy(ip,mensagem);
    SOCKET winsock;
    SOCKADDR_IN sock;
    winsock=socket(AF_INET,SOCK_DGRAM, 0);                                      //Informações sobre a conexão e o socket.
    sock.sin_family=AF_INET;
    sock.sin_addr.s_addr=inet_addr(IP_CLIENTE);
    sock.sin_port=htons(9010);
    sendto(winsock,ip,strlen(ip),0,(SOCKADDR*)&sock,sizeof(sock));              //Envia a resposta ao cliente.
}

void aguardarArquivos()                                                         //Função para aguardar o recebimento de arquivos.
{
    SOCKET s;
    struct sockaddr_in server, si_other;
    int slen, recv_len;
    char buf[BUFLEN];
    char mensagem[BUFLEN];
    WSADATA wsa;
    char *alvo;

    slen = sizeof(si_other);

    WSAStartup(MAKEWORD(2,2),&wsa);                                             //Inicialização do winsock.
    s = socket(AF_INET, SOCK_DGRAM, 0 );                                        //Cria o socket.

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;                                        //Informações sobre a conexão e o socket.
    server.sin_port = htons( PORT );

    bind(s,(struct sockaddr *)&server, sizeof(server));                         //Bind: Liga a porta ao socket na rede.
    printf("Aguardando arquivos...\n");


    while(1)                                                                    //Loop para recebimento de requisição.
    {
        fflush(stdout);

        memset(buf,'\0', BUFLEN);                                               //Limpa o buffer para recebimento de dados.
                                                                                //Prepara o programa para entrar em modo de recebimento.
        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
        {
            printf("Erro de recvfrom()!\n");
            returnClick();
        }

        char *ip_cliente = inet_ntoa(si_other.sin_addr);                         //Converte IPv4 em char.

        FILE *arq = fopen("bancoDeDados.txt", "a+");                             //Abre o arquivo banco de dados.txt.
        fprintf(arq, "%s %s\n", ip_cliente, &buf);                               //Mostra informações contidas no .txt na tela.
        printf("\nArquivo '%s' de: %s",&buf, ip_cliente);

        fclose(arq);				                                             //Fecha o arquivo.
    }

    close(s);                                                                    //Fecha o socket.
    WSACleanup();                                                                //Termina o uso da winsock2.dll.

    return 0;

}

int exibirBancoDeDados()                                                         //Abre o arquivo .txt com os arquivos do banco de dados.
{

    char ip[16], chave[32];
    FILE *arq = fopen("bancoDeDados.txt", "rt");
    printf("\n");
    while( (fscanf(arq, "%s %s\n", &ip, &chave)) != EOF)                         //Percorre o arquivo exibindo os dados.
    {
        printf("%s -> %s\n", &ip, &chave);

    }
    fclose(arq);                                                                 //Fecha o arquivo.txt.
    return;
}

// MAIN

int main()                                                                       //Função principal do programa.
{
    setlocale(LC_ALL, "Portuguese");                                             //Define o programa para português.
    menu();                                                                      //Função para exibição do menu principal do programa.
    return 0;
}
