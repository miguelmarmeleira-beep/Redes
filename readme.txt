NetBoX — fases I e II

Compilacao (Linux, GCC e make):
    make

Execucao:
    ./user -m 58001 [-n 193.136.138.142] [-p 59000]

-m e obrigatorio. As portas devem conter apenas digitos e estar entre 1 e
65535. -n recebe um endereco IPv4. Cada opcao pode aparecer apenas uma vez.
No laboratorio LT5, o enunciado indica o IP 192.168.1.1 para o DS.

Comandos:
    login UID password
    logout
    unregister
    publish filename label
    remove filename
    list
    versions filename
    exit

UID: 6 digitos. Password: 8 caracteres alfanumericos ASCII.
Filename: base nao vazia, ate 24 caracteres no total, extensao de exatamente
3 caracteres alfanumericos; base com letras, digitos, - ou _.
Label: 1 a 20 caracteres (letras, digitos, - e _).
O ficheiro a publicar deve ser regular, existir no diretorio de trabalho
e ter entre 0 e 10 000 000 bytes. Um diretorio nao pode ser publicado.
Fazer logout antes de iniciar outra sessao ou executar exit. Se logout ou
unregister confirmar que a sessao/registo ja nao existe no DS, a sessao local
e limpa. Erros de comunicacao ou password nao limpam a sessao local.

Limites atuais:
- Comandos ate 254 caracteres antes da quebra de linha.
- Respostas UDP ate 2047 bytes; cobre os 50 filenames previstos em RLS.
- Respostas TCP recebidas em blocos, com memoria alocada conforme necessario.
  A lista de versoes so e apresentada depois de validada na totalidade.
- Timeout UDP de 5 segundos; prazo total TCP de 5 segundos para ligar,
  enviar o pedido e receber a resposta completa.
- publication_time e mostrado tal como recebido, pois estes enunciados nao
  especificam o formato do campo.
- where, get e servidor TCP de peers pertencem a fases posteriores.

Limpar o executavel gerado:
    make clean
