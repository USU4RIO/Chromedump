# Chromedump v0.1

> Grava todas as senhas salvas pelo google chrome em um arquivo de texto
chamado Chromereport.txt. Casos de erros também estarão escrito dentro do 
arquivo. 

# Dependências

> Necessario a biblioteca sqlite3 (ja inclusa no repositório) para manipular o 
banco de dados onde o chrome armazena as senhas.
> Necessario incluir a biblioteca de criptografia do windows (wincrypt.h) no codigo
e na hora de compilar.

# Compilar o codigo

> Para compilar o codigo é necessario informar ao compilador o arquivo de codigo
da biblioteca sqlite3 (sqlite3.c) e a biblioteca de criptografia do windows (crypt32).
> Use: gcc Chromedump.c sqlite3.c -o Chromedump -lcrypt32 