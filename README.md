# feup-rcom
RCOM



	*** Aspetos importantes - 3ª aula ***
	
	1. Para os que ainda não implementaram o timeout -> trabalhar nisso
		implementação do timeout -> ficheiro alarm.c
		- ver exemplo de utilização da aula passada
		
		
		void alarmHandler(int sig) {
			// colocar aqui o código que deve ser executado
						
		}
		
		signal(SIGALRM, alarmHandler); 	// colocar na função main()
		alarm(TIMEOUT); 		// depois de enviar a trama SET por forma a lançar
								// um temporizador
				  
		
		Nota: 
			- código que forneci na aula passada foi apenas um exemplo
			- não sendo a solução mais recomendável, é possível executar ações 
			dentro do alarmHandler
				-> implica manutenção de múltiplas variáveis globais (fd, flags, ...)

				  
	2. Construção e organização do código
	(llopen(), llread(), llwrite(), llclose()) -> tal como está especificado
	no guião (slide 16)
	
	
	3. Construção e transmissão de tramas de informação, incluindo
	BCC1(cabeçalho) e BCC2 (dados)
		- cálculo do BCC2 -> XOR sucessivo dos bytes de dados
		- implementação do mecanismo Stop and Wait
		
		+-------+				+-------+
		|	S   |				|	R   |
		+-------+				+-------+
			|   I(Ns=0; C=0x00)		|
			|---------------------->|
			|	RR(Nr=1; C=0x85)	|
			|<----------------------|
			|						|
			|						|
			|    I(Ns=1; C=0x40)	|
			|---------------------->|
			|	RR(Nr=0; C=0x05)	|
			|<----------------------|
			|						|
	
		Nota: 
			- Campo C definido no slide 7 de acordo com o nr de sequência da Trama I ou RR
			- Slide 11 explica detalhadamente o procedimento que deve ser adotado pelo recetor

		
	4. Controlo de erros 
		- cálculo do BCC_1 e BCC_2 do lado do receptor e comparação
		com BCC_1 e BCC_2 extraídos da trama I -> ver slide 11
	
	
	5. Timeout para retransmissão de tramas I
		- há vários casos que podem ocorrer
			- recetor não recebe trama I 
			- recetor recebe trama com erros (detetados com base no BCC2)
			- emissor não recebe RR
			- ... 
		- necessidade manter uma cópia local enquanto o número de sequência Ns não
		se alterar
	
	
	6. Byte stuffing -> ver slides 12 e 13 
	
	
	7. Elementos de avaliação do trabalho -> ver slides 25 e 26
	