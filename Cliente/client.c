              /*
              Universidade Federal de Itajubá.
              COM240 - Redes de computadores.
              Professor Bruno Guazelli Batista.
              Programa para enviar arquivos via socket entre sistemas finais diferentes.
              Protocolo UDP.
              Implementado em C para windows.
              Contém soma de verificação, número de sequência, temporizador e confirmação de recebimento.
              Versão: Cliente.
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
#include <dirent.h>    //Biblioteca para utilizar os diretórios de arquivos do sistema.
#include <time.h>      //Biblioteca para manipular data e tempo.

#else

//Dá mensagem de erro informando que o sistemas não é suportado.
#error "Sistema nao suportado."

#endif

#pragma comment(lib,"ws2_32.lib")                                //Comando para utilizar a winsock.dll.
#define BUFLEN 512                                               //Tamanho máximo do buffer.
#define PORT 8888                                                //Porta pela qual serão passados os dados.

typedef struct input_data {                                      //Estrutura com informações da data e/ou tempo.
    char file_name[50];
    int size;
} INPUT_DATA;

typedef struct cabecalho{                                        //Estrutura com informações sobre número de sequência.
    int seqNum;
    int tam;
    int lim;
    unsigned char buff[256];                                     //Máximo tamanho permitido para um pacote.
    unsigned int soma;
} CABECALHO;

typedef struct acknowledgement{                                  //Estrutura com informações de confirmação de recebimento/emvio.
    int seqNum;
    int lim;
    unsigned int flag;                                           //Flag para confirmar recebimento do pacote.
} ACK;


// PROTÓTIPOS
int menu(char *IP_RAST);                                        //Função para o menu principal do programa.
void refresh();                                                 //Função para atualizar a frontend do programa.
void refreshAut(char *IP_RAST);                                 //Função para mostrar o menu após a conexão com o rastreador.
void returnClick();                                             //Função para retornar o programa para a página anterior.
int listarArq();                                                //Função para listar os arquivos presentes no cliente.
int semearArq();                                                //Função para semear os arquivos.
int requisitarArquivos(char *IP_RAST);                               //Função para requisitar um arquivo ao rastreador.
void transferirArquivos(char *mensagem, char *alvo, int opt);    //Função para transferir arquivos ao rastreador.
void atualizarBD(char *IP_RAST);                                      //Função para atualizar o banco de dados.
int aguardarResposta(char *IP_RAST);                            //Função para aguardar resposta do rastreador.
int main();                                                     //Função principal do programa.


// FUNÇÕES
int menu(char *IP_RAST)                                         //Inicia o menu do programa, com opções para o usuário escolher.
{
    printf(IP_RAST);
    int opcao=0;
    while(1)
    {
        refreshAut(IP_RAST);
        printf("\nOPCOES:\n");
        printf("   1- Requisitar Arquivo\n");
        printf("   2- Semear Arquivos\n");
        printf("   3- Listar Arquivos da pasta\n");
        printf("   4- Atualizar Banco de Dados\n");
        printf("   0- Fechar cliente\n\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);
        switch(opcao)
        {
        case 1:                                                 //Inicia a conexão e faz uma requisição ao rastreador.
            refreshAut(IP_RAST);
            requisitarArquivos(IP_RAST);
            break;
        case 2:                                                 //Inicia a conexão e começa a semear os arquivos que o cliente possui.
            refreshAut(IP_RAST);
            semearArq();
            returnClick();
            break;
        case 3:                                                 //Lista arquivos presentes no cliente.
            refreshAut(IP_RAST);
            listarArq();
            returnClick();
            break;
        case 4:                                                 //Atualiza banco de dados.
            refreshAut(IP_RAST);
            atualizarBD(IP_RAST);
            returnClick();
            break;
        case 0 :
            refreshAut(IP_RAST);                                //Realiza a saída do programa e exibe uma mensagem de conclusão.
            printf("\n\tSISTEMA CONCLUIDO\n");
            getch();
            exit(0);
            break;
        default :                                               //Mostra uma mensagem as demais opções não presentes no menu.
            refreshAut(IP_RAST);
            printf ("Valor invalido!\n");
            returnClick();
        }
    }
    return;
}

unsigned int checksum(void *buffer, size_t len)                 //Função para realizar a soma de verificação.
{
    unsigned int seed = 0;
    unsigned char *buf = (unsigned char *) buffer;
    size_t i;

    for (i = 0; i < len; ++i)                                   //Loop para aplicar a soma de verificação.
    {
        seed += (unsigned int) (*buf++);
    }

    return seed;
}

int createSocketUDP()                                           //Função para criação do socket.
{
    int sock;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == SOCKET_ERROR)//Cria o socket.
    {
        perror("Falha ao criar socket!\n");
        getch();
        exit(EXIT_FAILURE);
    }

    printf("\nSocket criado!\n");
    fflush(stdout);

    return sock;
}

void refresh()
{
    system("cls");
    printf("-----------------------------------------\n");
    printf("|\tTRANSFERENCIA DE ARQUIVOS MP3\t|\n");
    printf("-----------------------------------------\n");      //Design do cabeçalho do programa.
    printf("|\t\t CLIENTE     \t\t|\n");
    printf("-----------------------------------------\n");
}

void refreshAut(char *IP_RAST)
{
    system("cls");
    printf("-----------------------------------------\n");
    printf("|\tTRANSFERENCIA DE ARQUIVOS MP3\t|\n");
    printf("-----------------------------------------\n");
    printf("|\t\t CLIENTE     \t\t|\n");
    printf("-----------------------------------------\n");       //Limpa a tela e exibe a tela após a conexão do cliente com o rastreador.
    printf("|\tUtilizando o rastreador:\t|\n");
    printf("\t      %s \t\t\n", IP_RAST);
    printf("-----------------------------------------\n");
}

void returnClick()
{
    printf("\n\nAperte um botao para retornar...");              //Mostra uma mensagem as demais opções não presentes no menu.
    getch();
    refresh();
}

FILE *file_open(char *x)                                         //Função para abrir um arquivo.
{
    FILE *fp = fopen(x, "rb");

    if (!fp)
    {
        perror("Erro ao abrir o arquivo!");
        fclose(fp);
        exit(1);
    }

    return fp;
}

int file_size(FILE *fp)                                          //Função para analisar o tamanho do arquivo.
{
    int s;

    fseek(fp, 0L, SEEK_END);
    s = ftell(fp);
    printf("Tamanho do arquivo: %d bytes\n", s);

    return s;
}

int listarArq()                                                 //Função para listar os arquivos.
{
    struct dirent *de;                                          //Ponteiro para o diretório DIR.
    DIR *dr = opendir(".");                                     //opendir()retorna um ponteiro do tipo DIR.

    if (dr == NULL)                                             //Retorna Nulo se não encontrar diretório.
    {
        printf("\nDiretorio não pode ser aberto.\n" );
        return 0;
    }

    while ((de = readdir(dr)) != NULL)                          //Atualiza o diretório.
    {
        if( ((strcmp("client.exe", de->d_name))!=0) &&
                ((strcmp("client.o", de->d_name))!=0) &&
                ((strcmp("client.c", de->d_name))!=0) &&
                ((strcmp(".", de->d_name))!=0) &&
                ((strcmp("..", de->d_name))!=0) &&
                ((strcmp("...", de->d_name))!=0) &&
                ((strcmp("BD2", de->d_name))!=0)
          )
        {
            printf("%s\n", de->d_name);
        }
    }

    closedir(dr);                                               //Fecha o diretório.
    return 0;
}

int semearArq()                                                 //Função para semear um arquivo na rede.
{
    SOCKET sRec;
    struct sockaddr_in server, si_other;                    //Declara as variáveis a serem utlizadas na conexão.
    int slen, recv_len;
    char buf[BUFLEN];
    char mensagem[BUFLEN];
    WSADATA wsa;

    slen = sizeof(si_other);

    WSAStartup(MAKEWORD(2,2),&wsa);                             //Inicia o socket.
    sRec = socket(AF_INET, SOCK_DGRAM, 0 );                     //Cria o socket.

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;                      //Prepara a estrutura com as informações da conexão e do socket.
    server.sin_port = htons(8889);

    bind(sRec,(struct sockaddr *)&server, sizeof(server));  //Bind: Liga a porta ao socket na rede.
    printf("Semeando...\n");                                                   //Loop para recebimento de requisição.

    fflush(stdout);
    memset(buf,'\0', BUFLEN);                                     //Limpa o buffer para recebimento de dados.

                                                                  //Prepara o programa para entrar em modo de recebimento.
    if ((recv_len = recvfrom(sRec, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
    {
        printf("Erro de recvfrom()!\n");
        returnClick();
        return;

    }

    printf("\nRequisição recebida: %s\n", buf);
    char *IP_CLIENTE = inet_ntoa(si_other.sin_addr);             //Realiza a transferância.
    transferirArquivos(buf, IP_CLIENTE, 0);

    close(sRec);                                                   //Fecha o socket.
    WSACleanup();                                                  //Termina o uso da winsock2.dll.
}

int requisitarArquivos(char *IP_RAST)
{
    struct sockaddr_in server, si_other;
    int s, slen=sizeof(si_other);
    char buf[BUFLEN];
    char mensagem[BUFLEN];
    char alvo[16];
    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)                        //Inicializa o winsock.
    {
        printf("Erro ao inicializar Winsock!\n");
        returnClick();
    }
    printf("Winsock Inicializado.\n");


    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) //Cria o socket.
    {
        printf("Erro de socket()!\n");
        returnClick();
    }


    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);                                //Informações sobre conexão e o socket.
    si_other.sin_addr.S_un.S_addr = inet_addr(IP_RAST);


    printf("Requisitar arquivo: ");
    scanf("%s", &mensagem);


    if (sendto(s, mensagem, strlen(mensagem), 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR) //Envia a mensagem.
    {
        printf("Erro de sentTo()!\n");
        returnClick();
    }

    strcpy(alvo,aguardarResposta(IP_RAST));                         //Analise do banco de dados.
                                                                    //Confirma se há ou não o arquivo.
    if((strcmp("false", alvo))==0)
    {
        printf("\nArquivo nao encontrado.");
        returnClick();
    }
    if((strcmp("false", alvo))!=0)
    {
        printf("\nServidor: arquivo '%s' encontrado no cliente %s\n", mensagem, alvo);
        printf("\tComecar a transferencia? (1-Sim/0-Nao)\n\t");
        int transf;
        scanf("%d", &transf);
        if(transf == 1)
        {
            printf("\nSolicitando ao IP %s...", alvo);
            solicitarArquivos(mensagem, alvo);
        }
        else
        {
            printf("\nTransferencia nao realizada\n");
            returnClick();
        }
    }

    close(s);                                                           //Fecha o socket.
    WSACleanup();

}

int solicitarArquivos(char *mensagem, char *alvo)                        //Função para solicitar arquivos para o rastreador.
{
    struct sockaddr_in server, si_other_2;
    int s, slen=sizeof(si_other_2);
    char buf[BUFLEN];
    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)                            //Inicializa o winsock.
    {
        printf("Erro ao inicializar Winsock!\n");
        returnClick();
    }
    printf("Winsock Inicializado.\n");


    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)  //Cria o socket.
    {
        printf("Erro de socket()!\n");
        returnClick();
    }

    memset((char *) &si_other_2, 0, sizeof(si_other_2));
    si_other_2.sin_family = AF_INET;
    si_other_2.sin_port = htons(8889);                                  //Informações sobre o socket e a conexão.
    si_other_2.sin_addr.S_un.S_addr = inet_addr(alvo);


    if (sendto(s, mensagem, strlen(mensagem), 0, (struct sockaddr *) &si_other_2, slen) == SOCKET_ERROR)  //Envia a mensagem.
    {
        printf("Erro de sentTo()!");
        returnClick();
    }
    transferirArquivos(mensagem, alvo, 1);
    return;

}

void send_file(FILE *arq, int sock, struct sockaddr_in si_other)         //Função para enviar um arquivo.
{
    ACK ACK_msg;
    CABECALHO cabecalho;
    int slen = sizeof (si_other);
    int read_file = 1;

    memset(cabecalho.buff, 0, sizeof (cabecalho.buff));
    fseek(arq, 0L, SEEK_SET);                                //Seta o ponteiro para o inicio do arquivo.

    printf("Arquivo sendo enviado...\n");
    int seqNum = 0;
    int id = 0;
    int i = 0;
    int rval;
    int sends = 0;

    while (!feof(arq))                                       //Início do loop para enviar o arquivo.
    {

        if (read_file == 1)                            
        {
            cabecalho.lim = 0;
            for (i = 0; i < sizeof (cabecalho.buff); i++)
            {
                fread(&(cabecalho.buff[i]), 1, 1, arq);
                if (feof(arq))
                {
                    cabecalho.buff[i] = '\0';
                    break;
                }
            }
        }
        else
        {
            ACK_msg.lim = ++cabecalho.lim;
            cabecalho.seqNum = --seqNum;                    //Decresce o numero de sequência.
            if (cabecalho.lim <=5)                          //Temporizador.
            {
                printf("\n%d tentativas\n", ACK_msg.lim);


                fseek(arq, (cabecalho.seqNum)*100, SEEK_SET); //Ponteiro do arquivo para pacote perdido.

                for (i = 0; i < sizeof (cabecalho.buff); i++)
                {
                    fread(&(cabecalho.buff[i]), 1, 1, arq);
                    if (feof(arq))
                    {
                        cabecalho.buff[i] = '\0';
                        break;
                    }
                }
            }
        }
        if (i != 0)
        {
            cabecalho.soma = checksum(cabecalho.buff, i);    //Cálculo da soma de verificação.
            cabecalho.tam = i;
            cabecalho.seqNum = ++seqNum;                    //Adiciona o numero de sequencia do pacote.
            printf("\nMSG Enviada: Id:%d ", cabecalho.seqNum);
            fflush(stdout);
            printf("tamanho:%d ", cabecalho.tam);
            fflush(stdout);
            printf("checksum:%d\n", cabecalho.soma);
            fflush(stdout);

enviando:                                                   //Realiza a transferencia caso nao ocorra erro.
            if (sendto(sock, &cabecalho, sizeof (cabecalho), 0,
                       (struct sockaddr *) &si_other, slen) <= 0)
            {
                perror("Falha ao enviar!!!");
                fclose(arq);
                close(sock);                                //Caso ocorra erro o socket e fechado.
                exit(1);
            }

            fd_set select_fds;                              //Pipe usado por select.
            struct timeval timeout;                         //Tempo limite.


            FD_ZERO(&select_fds);                           //Limpando o pipe.
            FD_SET(sock, &select_fds);                      //Configurando o socket.

            timeout.tv_sec = 1;                             //Tempo limite configurado em 5 segundos.
            timeout.tv_usec = 0;

            printf("Esperando pela mensagem...\n");
            fflush(stdout);


            if (cabecalho.lim == 5)           //Tentativas de enviar o pacote será igual a variavel tentativas.
            {
                printf("\nO Falha na transferencia!\nTemporizador estourou o tempo limite estipulado!\n\n");
                fflush(stdout);
                fclose(arq);
                close(sock);
                exit(1);
            }

                                                                    //Temporizador de 5 segundos.
            if (select(32, &select_fds, NULL, NULL, &timeout) == 0)  //se estourar o tempo limite
            {
                if(sends < 5)                                       //Faz 5 tentativas.
                {
                    ++sends;
                    printf("\nReenviando arquivo, %d tentativas\n", sends);
                    fflush(stdout);
                    goto enviando;
                }
                                                                    //Perda de conexão devido ao tempo.
                printf("\nFalha ao enviar o arquivo!\nTemporizador estourou o tempo limite estipulado!\n\n");
                fflush(stdout);
                fclose(arq);
                close(sock);
				getch();                                        //Fecha o socket por estouro de temporizador.
                exit(1);
            }
            else                                                    //Envio do arquivo.
            {
                sends = 0;
                rval = recvfrom(sock, &ACK_msg, sizeof (ACK_msg), 0,
                                (struct sockaddr *) &si_other, &slen);
            }

            if (rval < 0)
            {
                perror("Erro ao ler fluxo de mensagens!\n");
            }
            else if (rval == 0)
            {
                printf("Conexao finalizada!\n");
                fflush(stdout);
            }
            else                                                //Se não ocorrer erro, recebe confimação.
            {
                printf("ACK %d RECEBIDO.\n", ACK_msg.seqNum);
            }


            if (seqNum == ACK_msg.seqNum && ACK_msg.flag == 0)  //Veriica se não houve erro na mensagem de confirmação.
            {
                read_file = 1;
            }
            else
            {
                read_file = 0;
            }
        }
    }
}

void transferirArquivos(char *nome_arq, char *alvo, int opt)            //Função para realizar a transferência dos arquivos.
{
    if(opt==0)
    {
                                                                        //Envia um arquivo.
        int sock;
        struct sockaddr_in client;
        FILE *fp;
        struct hostent *hp;
        char buff[10] = "/0";
        int rval;
        int count = 0;
        int sz;
        struct sockaddr_in si_me;
        int slen = sizeof (client);
        WSADATA wsa;

        fp = file_open(nome_arq);
        sz = file_size(fp);


        INPUT_DATA id;
        strcpy(id.file_name, nome_arq);                         //Aloca os dados sobre a transferência em uma estrutura.
        id.size = sz;
        if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)                //Incicia o Winsock.
        {
            printf("Erro ao inicializar Winsock!");
            returnClick();
        }
        if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)  //Cria o socket.
        {
            perror("Falha ao criar socket!\n");
            returnClick();
        }

        client.sin_family = AF_INET;                                            //Configuração do socket.

        if (strcmp(alvo, IP_RAST) == 0)
        {

            hp = gethostbyname(alvo);                                           //Descobrir IP do hospedeiro pelo nome.
            if (hp == 0)
            {
                perror("Falha ao conectar endereco IP!");
                fclose(fp);
                close(sock);
                exit(1);
            }
            memcpy(&client.sin_addr, hp->h_addr, hp->h_length);                 //Copia um endereço de memória.
        }
        else
        {
            client.sin_addr.s_addr = inet_addr(alvo);                           //Estabelece conexão entre hospedeiros diferentes.

        }

        client.sin_port = htons(9005);                                          //Estabelece comunicação entre o socket e a porta informada.


        if (sendto(sock, &id, sizeof (id), 0, (struct sockaddr *) &client, slen) <= 0)  //Inicia o envio dos dados.
        {
            perror("Falha ao enviar arquivo!");
            fclose(fp);
            close(sock);
            exit(1);
        }

        printf("\nChecando conexão: enviando %s ", id.file_name);
        fflush(stdout);
        printf("Tamanho: %d bytes\n", id.size);
        fflush(stdout);

        send_file(fp, sock, client);                                            //Envia o arquivo.

        printf("\nArquivo enviado com sucesso!\n");
        fflush(stdout);
        fclose(fp);
        close(sock);
    }
    else
    {

                                                                                //Recebe um arquivo.
        INPUT_DATA id;
        struct sockaddr_in server;
        int sock;
        int mysock;
        char buff[5];
        int rval;
        int count = 0;                                                          //Informações e variáveis para a soma de vericação e o recebimento.
        FILE *fp1;
        CABECALHO cabecalhos;
        ACK ACK_msg;
        int slen = sizeof (server);
        int seqNum = 1;
        int flag = 0;
        WSADATA wsa;

        if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)                                //Inicia o winsock.
        {
            printf("Erro ao inicializar Winsock!\n");
            returnClick();
        }
        if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)  //Cria o socket.
        {
            perror("Falha ao criar socket!\n");
            returnClick();;
        }

        printf("\nSocket criado!\n");
        fflush(stdout);

        {

            memset((void *) &server, 0, sizeof (server));
            server.sin_family = AF_INET;
            server.sin_port = htons(9005);                                        //Inormações sobre a conexão e o socket.
            server.sin_addr.s_addr = htonl(INADDR_ANY);


            if (bind(sock, (struct sockaddr *) &server, sizeof (server)) == -1)   //Estabelece conexãp entre o socket e a porta informada.
            {
                perror("Falha ao conectar a porta ao socket!");
                close(sock);
                exit(-1);
            }
        }

        printf("Aguardando conexao!!!\n\n");
        fflush(stdout);


        mysock = sock;
        if ((rval = recvfrom(mysock, &id, sizeof (id), 0,
                             (struct sockaddr *) &server, &slen)) < 0)              //Envia os dados do cabeçalho.
        {
            perror("ERRO ao ler o fluxo de mensagens!\n");
        }
        else if (rval == 0)                                                         //Finaliza a conexão.
        {
            printf("Finalizando conexÃ£o!!!\n");
            fflush(stdout);
        }
        else
        {
            printf("MSG recebida -->[arquivo]: %s ", id.file_name);
            fflush(stdout);
            printf("- [tamanho]: %d bytes\n", id.size);
            fflush(stdout);
        }

        memset(buff, 0, sizeof (buff));
        printf("Copia do arquivo iniciada!!!\n\n");
        fflush(stdout);
        fp1 = fopen(id.file_name, "wb");                                            //Realiza a cópia do arquivo.
        int read_bytes = 0;


        while (read_bytes < id.size)                                                //Leitura dos bytes.
        {
            fd_set select_fds;
            struct timeval timeout;                                                 //Valor do tempo limite.

            FD_ZERO(&select_fds);                                                   //Limpa os pipes.
            FD_SET(mysock, &select_fds);                                            //Conigura o socket.


            timeout.tv_sec = 5;                                                     //Tempo limite em 5 segundos.
            timeout.tv_usec = 0;

            printf("Esperando mensagem!!!\n");
            fflush(stdout);

            if (select(32, &select_fds, NULL, NULL, &timeout) == 0)  //Espera pela mensagem dentro do tempo limite.
            {

                printf("\nFalha na transferencia de arquivo!\nTemporizador estourou o tempo limite estipulado!\n\n");   //Temporizador com 5 segundos.
                fflush(stdout);
                fclose(fp1);
                close(mysock);
                exit(1);
            }
            else
            {
                rval = recvfrom(mysock, &cabecalhos, sizeof (cabecalhos), 0,
                                (struct sockaddr *) &server, &slen);                //Salva os dados do cabeçalho.
            }

            if (rval < 0)
            {
                perror("Erro ao ler o fluxo de mensagens!\n");
            }
            else if (rval == 0)
            {
                printf("Conexao finalizada!\n");
                fflush(stdout);
            }
            else
            {
                unsigned int x = checksum(cabecalhos.buff, cabecalhos.tam);         //Realiza a soma de veriicação.


                printf("Checksum calculado : %d\n", x);
                fflush(stdout);
                printf("MSG valida %d recebida: ", cabecalhos.seqNum);
                fflush(stdout);
                printf("tamanho: %d ", cabecalhos.tam);
                fflush(stdout);
                printf("checksum: %d\n", cabecalhos.soma);
                fflush(stdout);

                ACK_msg.seqNum = seqNum;                                            //Número de sequência.
                ACK_msg.lim = cabecalhos.lim;                                       //Temporizador.

                if ((x == cabecalhos.soma) && (cabecalhos.seqNum == seqNum))        //Envia o pacote pelo numero de sequencia.
                {
                    ACK_msg.flag = 0;
                    ++seqNum;

                    printf("Enviando ACK %d\n", ACK_msg.seqNum);
                    fflush(stdout);                                                 //Envia conirmação ACK.
                    if ((rval = sendto(mysock, &ACK_msg, sizeof (ACK_msg), 0,
                                       (struct sockaddr *) &server, slen)) < 0)     //Envia dados do cabeçalho.
                    {
                        perror("Erro ao ler o fluxo de mensagens!\n");
                    }
                    else if (rval == 0)
                    {
                        printf("Conexao finalizada!\n");                            //Finaliza conexão.
                        fflush(stdout);
                    }
                    else
                    {
                        fwrite(&cabecalhos.buff[0], cabecalhos.tam, 1, fp1);        //Grava os pacotes no arquivo.
                        printf("ACK ENVIADO.\n\n");
                        fflush(stdout);
                        read_bytes += cabecalhos.tam;
                    }

                }
                else                                                                //Se houver um erro.
                {
                    ACK_msg.flag = 1;
                    printf("Pacote perdido!\n");
                    fflush(stdout);
                    ACK_msg.lim;
                    ACK_msg.seqNum -= 1;                                            //Decresce o numero de sequencia.
                    if (ACK_msg.lim <= (5/*Tentativas*/ - 1))
                    {
                        printf("Reenviando ACK %d - ", ACK_msg.seqNum);
                        fflush(stdout);
                        printf("%d tentativas\n", ACK_msg.lim + 1);
                        fflush(stdout);
                        if ((rval = sendto(mysock, &ACK_msg, sizeof (ACK_msg), 0,
                                           (struct sockaddr *) &server, slen)) < 0)  //Reenvia dados do cabeçalho.
                        {
                            perror("Erro ao ler o fluxo de mensagens!\n");
                        }
                        else if (rval == 0)
                        {
                            printf("Conexao finalizada!\n");
                            fflush(stdout);
                        }
                        else
                        {
                            printf("ACK reenviado\n\n");
                            fflush(stdout);
                        }
                    }
                    else
                    {
                        printf("\nFalha na transferencia de arquivo!\nTemporizador estourou o tempo limite estipulado!\n");     //Estouro de temporizador.
                        fflush(stdout);
                        fclose(fp1);
                        close(mysock);
                        return 0;
                    }
                }

            }
        }

        printf("Cópia do arquivo completada com sucesso!\n");
        fflush(stdout);
        fclose(fp1);
        close(mysock);
        returnClick();
    }
}

void atualizarBD(char *IP_RAST)                               //Função para atualizar o banco de dados do rastreador.
{
    struct sockaddr_in server, si_other;
    int s, slen=sizeof(si_other);
    char buf[BUFLEN];
    char mensagem[BUFLEN];
    char alvo[16];
    WSADATA wsa;
    struct dirent *de;                                  //Ponteiro para o diretório de entrada.


    printf("\nO banco de dados deve ser atualizado? (1-Sim/0-Nao)\n");
    int transf;
    scanf("%d", &transf);
    if(transf == 1)
    {
        printf("\nO banco de dados sera atualizado...");
        Sleep(1000);
        printf("\nO banco de dados foi atualizado!");
    }
    else
    {
        printf("\nAtualizacao cancelada.");
        return;
    }

    WSAStartup(MAKEWORD(2,2),&wsa);                 //Inicia o winsock.
    s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);     //Cria o socket.

    memset((char *) &si_other, 0, sizeof(si_other)); //Informações sobre a conexão e o socket.
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    si_other.sin_addr.S_un.S_addr = inet_addr(IP_RAST);

                                                    //Começa a comunicação.

                                                    //opendir() retorna um ponteiro do tipo DIR.
    DIR *dr = opendir(".");

    if (dr == NULL)                                 //opendir retorna NULL se não conseguir abrir o diretório.
    {
        printf("\nNao existe nada no diretorio.\n" );
        return 0;
    }

    while ((de = readdir(dr)) != NULL)
    {
        if( ((strcmp("client.exe", de->d_name))!=0) &&
                ((strcmp("client.o", de->d_name))!=0) &&
                ((strcmp("client.c", de->d_name))!=0) &&
                ((strcmp(".", de->d_name))!=0) &&
                ((strcmp("..", de->d_name))!=0) &&
                ((strcmp("...", de->d_name))!=0) &&
                ((strcmp("BD2", de->d_name))!=0)
          )
        {

            sendto(s, de->d_name, strlen(de->d_name), 0, (struct sockaddr *) &si_other, slen);

        }
    }

    closedir(dr);
    return 0;
}

int aguardarResposta(char *IP_RAST)                             //Função para aguardar resposta do rastreador.
{
    SOCKET winsock;
    SOCKADDR_IN sock;
    int size=sizeof(sock);
    char mensagem[BUFLEN];

    winsock=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);             //Informações sobre a conexão e o socket.
    sock.sin_family=AF_INET;
    sock.sin_port=htons(9010);
    sock.sin_addr.s_addr = INADDR_ANY;
    bind(winsock,(SOCKADDR*)&sock,sizeof(sock));

    memset(mensagem,0,sizeof(BUFLEN));
    recvfrom(winsock,mensagem,BUFLEN,0,(SOCKADDR*)&sock,&size);
    closesocket(winsock);
    WSACleanup();
    return mensagem;
}

// MAIN

int main()
{
    setlocale(LC_ALL, "Portuguese");            //Função para definir o idioma para português.
    refresh();                                  //Exibe o cabeçalho do programa.
    printf("\nDigite o ip do servidor: ");
    char *IP_RAST;
    scanf("%s", &IP_RAST);                      //Leitura do IP do rastreador.
    menu(&IP_RAST);                             //Função para apresentar o menu.
    getch();                                    //Função para prorrogar o encerramento do programa.
    return 0;
}
