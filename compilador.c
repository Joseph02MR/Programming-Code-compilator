#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "structs.h"

#define false 0
#define true 1
#define TILDE 96

//prototipos
FILE *abrir_archivo (char name[255]);
char *leer_archivo (FILE *archivo);
int validar_char (char caracter);
char *procesar_entrada(char *entrada, unsigned long tam_entrada);
char *tokenizar_cod(char *codigo, struct Tabla_s *tabla);
void concat_buffer(char buffer[],char c);
char *convertir_op(int op_ptr);
char *convertir_delim(char delim);
void actualizar_salida(char *salida, int token_id, struct Tabla_s *tabla, char delim);
char *tokenizar_cod(char *codigo, struct Tabla_s *tabla);
char *parsear_tokens(char *tokens, struct Tabla_s *tabla);

int eval_sentencia(char buffer[][6], int buffer_size, struct Tabla_s *tabla); 
int eval_token_type(char *buffer);
char *get_token_type(char *buffer);
char *get_token_value(char *buffer, struct Tabla_s *tabla);
int get_token_id(char *buffer);

char *eval_expr(char buffer[CHAR_MAX][5], int buffer_ptr, struct Tabla_s *tabla);
int eval_pres(char *buffer, struct Tabla_s *tabla);
char *procesar_expr(char *expr, struct Tabla_s *tabla);

int eval_buffer(char buffer[], struct Tabla_s *tabla); 
int eval_sim(char actual, char proximo, int buffer_no_vacio, char anterior, char pos_anterior);
int eval_negativo(char proximo, char anterior, char pos_anterior);
int eval_num(char buffer[], struct Tabla_s *tabla); 	//funciones de automatas 
int eval_string(char buffer[], struct Tabla_s *tabla);
int eval_res(char buffer[], struct Tabla_s *tabla);
int eval_id(char buffer[], struct Tabla_s *tabla);
int eval_op(char buffer[], struct Tabla_s *tabla);
int eval_dec_as(char buffer[][6], int buffer_size, struct Tabla_s *tabla);


//constantes y globales
const char ALFABETO[] = " \r\n\t,;:.{}()abcdefghijklmnopqrstuvwxyz0123456789+-*/%#=\\\"'¡¿?|^<>!ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char RESERVADAS[][8] = {"byte", "bool", "break", "case", "char", "continue", "default", "do", "double", "else", 
"false", "float","for", "fun", "if", "in", "int", "long", "null", "short", "string", "switch", "return", "true","void", "while"};
const int RES_SIZE = 26;

const char DELIM[] = {TILDE,'{', '}','(',')',';', ','};
const char SIM_OP[] = {'+','-','/','*','%','^', '=','.','&','|','<','>','!', ':'};
const char OPS[][3] = {"+", "-", "/", "*", "%", "^", "=", "<", ">",
	"&", "|", "!", "==", "!=", ">=", "<=", "..", "...",":"};

const int OPS_SIZE = 19;

struct Tabla_s T_GLOBAL = {.tupla_cnt = 0};

int main (int argc, char** argv){
	FILE *archivo = abrir_archivo("archivo.txt"); //TODO: pasar nombre como parametro
	//analisis lexico del codigo
	char *codigo = leer_archivo(archivo);
	printf("%s\n",codigo);
	char *tokenizado = tokenizar_cod(codigo, &T_GLOBAL);
	if(tokenizado == NULL) return 1;
	
	printf("\n%s\n",tokenizado);
	
	//análisis sintáctico/semántico
	char *analizado = parsear_tokens(tokenizado, &T_GLOBAL);
	//if(analizado == NULL) return 1;
	//resto del codigo...
	
	printf("\nTABLA DE SIMBOLOS FINAL");
	printf("\n%3s %3s %8s %5s %3s %s", "ID", "TT", "TOK VAL", "DAT T", "VV", "EXPRESION");
	for(int i = 0; i < T_GLOBAL.tupla_cnt; i++){
		printf("\n%3i %3s %8s %5s %3i %s",T_GLOBAL.atributos[i]->id,
			T_GLOBAL.atributos[i]->tipo,T_GLOBAL.atributos[i]->valor,
			T_GLOBAL.atributos[i]->t_dato,T_GLOBAL.atributos[i]->valor_var,T_GLOBAL.atributos[i]->expr);
	}	
	return 0;
}

FILE *abrir_archivo (char name[CHAR_MAX]){
	FILE *archivo;
	if(!(archivo = fopen(name, "r"))){
		printf("\nError al abrir el archivo '%s'", name);
		exit (1);
	} 
	else return archivo;
}

/*lee el archivo y se verifica que no haya caracteres inválidos
	retorna una cadena con el contenido del archivo, 
	identifica también comentarios y los ignora.
*/
char *leer_archivo(FILE* archivo){
	char c;
	fseek(archivo, 0L, SEEK_END);
	unsigned long tam = ftell(archivo);
	rewind(archivo);
	
	char *contenido = malloc(tam * sizeof(char));
	int cont_aux = 0;
	int is_comment = false;

	while(true){
		c = fgetc(archivo);
		if(feof(archivo)) break;
		if(validar_char(c)){
			//printf("%c", c);
			//se ignoran los comentarios en el buffer
			if(c == '#'){
				is_comment = is_comment ? false : true;
				continue;
			}
			if(!is_comment) contenido[cont_aux++] = c;
			else continue;	
		}		
		else break;
	}
	contenido[cont_aux] = '\0';           
	fclose(archivo);

	return procesar_entrada(contenido, tam);
}  //a partir de aqui se puede considerar al buffer un string

int validar_char(char caracter){
	if(NULL != strchr(ALFABETO, caracter)){
		return true;
	}else {
		printf("\nError en el archivo. Caracter no definido");
		return 0;
	}
}

//convierte el texto plano en cadenas separadas por tildes eliminando "blanks" extra
char *procesar_entrada(char *entrada, unsigned long tam_entrada){
	char *copia = malloc(tam_entrada * sizeof(char));
	int is_blank = false;
	int is_string = false;
	int printable = false;
	unsigned long i = 0;
	unsigned long j = 0;
	
	while(entrada[i] != '\0'){
		if((entrada[i] == '"' || entrada[i] == '\'') && entrada[i-1] != '\\' ) {
			is_string = is_string ? false : true;
		}
		if(!is_string){
			if(entrada[i] == ' ' || entrada[i] == '\r' || entrada[i] == '\t' || entrada[i] == '\n'){
				if(!is_blank){
					copia[j] = TILDE;
					is_blank = true;
					j++;
				}
			}
			else printable = true;
		}
		else printable = true;
		if(printable){
			if(is_blank) is_blank = false;
			copia[j] = entrada[i];
			printable = false;
			j++;
		}
		i++;
	}
	copia[j] = '\0';
	free(entrada);
	return copia;	
} //despues de este metodo solo se permiten espacios o caracteres de escape dentro de comillas (strings)

char *tokenizar_cod(char *codigo, struct Tabla_s *tabla){
	int i = 0;
	int is_end = false;
	int is_string = false;
	int is_concat_on = false;
	
	int token_id = 0;
	int token_id_op = 0;
	int update = false;
	int concat = false;
	
	char buffer[SHRT_MAX] = "";
	char buffer_op[6] = "";
	char *salida = malloc(strlen(codigo)*3 *sizeof(char));
	salida[0] = '\0';
	
	
	while ((codigo[i] != '\0' || buffer[0] != '\0' || buffer_op[0] != '\0')){
		
		if(codigo[i] == '\0') {
			/*corregir eval_todo 
			*/
		}
		if(codigo[i] == '"' && codigo[i-1] != '\\') is_string = is_string ? false : true;
		
		if(is_string) is_concat_on = true; //add_buffer
		else{
			if(NULL != strchr(DELIM, codigo[i]) || NULL != strchr(SIM_OP, codigo[i])){
				
				int is_op = false;
				//evalua si el punto es parte de un decimal o un operador
				if(eval_sim(codigo[i], codigo[i+1], buffer_op[0] != '\0', codigo[i-1], codigo[i-2])){
					if(buffer[0] != '\0'){
						token_id = eval_buffer(buffer, tabla);
						if(token_id == false){
							printf("\nERROR, token no valido: %s",buffer);
							return NULL;
						}
						buffer[0] = '\0'; //vaciar buffer
						actualizar_salida(salida, token_id,tabla, codigo[i]);
					}
					check_op:
					if(NULL != strchr(SIM_OP, codigo[i]) && 
					eval_sim(codigo[i], codigo[i+1], buffer_op[0] != '\0', codigo[i-1], codigo[i-2])) 
						concat_buffer(buffer_op,codigo[i]);//add_buffer_op;
					else {
						if(buffer_op[0] != '\0' ){
						 	token_id_op = eval_op(buffer_op, tabla);
						 	if(token_id_op == false){
								printf("\nERROR, token no valido (OP): %s",buffer_op);
								return NULL;
							}
						 	buffer_op[0] = '\0'; //vaciar buffer
						 	actualizar_salida(salida, token_id_op, tabla, codigo[i]);	
						}
						else if(!is_concat_on) actualizar_salida(salida, 0, tabla, codigo[i]);
						//else do nothing
					}
				}
				else concat = true; //add_buffer
			}
			else concat = true; //add_buffer
		}
		if(concat){
			is_concat_on = true;
			concat = false;
			goto check_op;
		}
		if(is_concat_on){
			concat_buffer(buffer,codigo[i]); 
			is_concat_on = false;
		}
		i++;
	}
	free(codigo);
	return salida;
}

/*se verifican los casos especiales de los caracteres . y -
 se revisa si el punto es parte de un numero o de un operador
 */
int eval_sim(char actual, char proximo, int buffer_no_vacio, char anterior, char pos_anterior){
	if(actual != '.' && actual != '-') return true;
	if(actual == '-') return eval_negativo(proximo, anterior, pos_anterior);
	else if(actual == '.' && (proximo == '.' || buffer_no_vacio)) return true;
	else return false;
}

//se verifica si el signo - es parte de un numero o es un operador (evaluación semántica)
int eval_negativo(char proximo, char anterior, char pos_anterior){
	if(isdigit(proximo) && (!isdigit(anterior) || !isdigit(pos_anterior))) return false;
	else return true;
}

/*se verifica el contenido del buffer en el sig. orden: número, string/char, keyword, identificador
	de lo contrario retorna error
*/
int eval_buffer(char buffer[], struct Tabla_s *tabla){
	if(isdigit(buffer[0]) || buffer[0] == '-') return eval_num(buffer, tabla);
	else{
		int aux=0;
		if(aux=eval_string(buffer, tabla)) return aux;
		else if(aux=eval_res(buffer, tabla)) return aux;
		else if(aux=eval_id(buffer, tabla)) return aux;
		else return false;
	}
}

int eval_num(char buffer[], struct Tabla_s *tabla){
	char N[] = "123456789";
	int status = 1; 
	int is_signed = false;
	int has_point = false;
	for(int i = 0; i < strlen(buffer); i++){
		if(buffer[i] == '.'){
			if(is_signed) return false;
			else is_signed = true;
		}
		if(buffer[i] == '-'){
			if(has_point) return false;
			else has_point = true;
		}
		if(isdigit(buffer[i]) || (!isdigit(buffer[i])&&(buffer[i]=='.'||buffer[i]=='-'))) {
			if(status == 1 && buffer[i] == '-') status = 2;
			else if((status == 1 || status == 2) && buffer[i] == '0') status = 3;
			else if((status == 1 || status == 2) && NULL != strchr(N,buffer[i])) status = 4;
			else if( status == 4 && NULL != strchr(N,buffer[i])) continue;
			else if( (status == 3 || status == 4) && buffer[i] == '.') status = 5;
			else if( (status == 5 || status == 6) && (buffer[i] == '0' || NULL != strchr(N,buffer[i]))) status = 6;	
			else break;
		}
		else return false;
	}
	switch(status){ 
		case 3:	//zero
		case 4: return agregar_token(tabla, crear_tupla("IN", buffer)); 
			break;
		case 6: return agregar_token(tabla, crear_tupla("FL", buffer)); 
			break;
		default: return false;
	}
}
int eval_string(char buffer[], struct Tabla_s *tabla){
	const char ESC[] = {'r','t','n','"','\'','\\','?'};
	int status = 1;
	for(int i = 0; i < strlen(buffer); i++){
		if(status == 1 && buffer[i] == '"') status = 2;
		else if(status == 2 && (buffer[i] != '"' || buffer[i-1] == '\\')  ) continue;
		else if(status == 2 && (buffer[i] == '"' && buffer[i-1] != '\\')) status = 3; // ? xd
		else if(status == 1 && buffer[i] == '\'') status = 4;
		else if(status == 4 &&  buffer[i] == '\\') status = 5;
		else if(status == 4 && buffer[i] != '\\') status = 6;
		else if(status == 5 && NULL != strchr(ESC, buffer[i])) status = 6;
		else if(status == 6 && buffer[i] == '\'') status = 7;
		else break;
	}
	switch(status){
		case 3: return agregar_token(tabla, crear_tupla("ST", buffer)); 
			break;
		case 7: return agregar_token(tabla, crear_tupla("CH", buffer));
			break;
		default: return false;
	}
}
int eval_res(char buffer[], struct Tabla_s *tabla){
	for(int i = 0; i < RES_SIZE; i++){
		if(strcmp(buffer,RESERVADAS[i])==0){
			char aux[8] = "";
			strcpy(aux,RESERVADAS[i]);
			return agregar_token(tabla, crear_tupla("KW", aux)); 
		}
	}
	return false;
}
int eval_id(char buffer[], struct Tabla_s *tabla){
	int status = 1;
	for(int i = 0; i < strlen(buffer); i++){
		if(status == 1 && (isalpha(buffer[i]) || buffer[i] == '_')) status = 2;
		else if(status == 2 && (isalpha(buffer[i]) || buffer[i] == '_' || isdigit(buffer[i]))) continue;
		else break;
	}
	if(status == 2){
		return agregar_token(tabla, crear_tupla("ID",buffer));
	}
	else return false;
}
int eval_op(char buffer_op[], struct Tabla_s *tabla){
	for(int i = 0; i < OPS_SIZE; i++){
		if(strcmp(OPS[i],buffer_op) == 0){
			return agregar_token(tabla, crear_tupla("OP",convertir_op(i)));
		}
	}
	return false;
}

void concat_buffer(char buffer[],char c){
	char aux[]={c,'\0'};
	strcat(buffer,aux);
	return;
}

//{"+", "-", "/", "*", "%", "^", "=", "<", ">", "&", "|", "!", "==", "!=", ">=", "<=", "..", "...", ":"}
char* convertir_op(int op_ptr){
	switch(op_ptr){
		case 0: return "ADD";
		case 1: return "SUS";
		case 2: return "DIV";
		case 3: return "MUL";
		case 4: return "MOD";
		case 5: return "EXP";
		case 6: return "AS";
		case 7: return "LT";
		case 8: return "MT";
		case 9: return "AND";
		case 10: return "OR";
		case 11: return "NOT";
		case 12: return "EQU";
		case 13: return "NEQ";
		case 14: return "ME";
		case 15: return "LE";
		case 16: return "RI";
		case 17: return "RE";
		case 18: return "TYP";
		default: return "ERR";
	}
}
 
 //recibe el id de los nuevas entradas en la tabla y las imprime en forma de token en la cadena de salida
void actualizar_salida(char *salida, int token_id, struct Tabla_s *tabla, char delim){
	char aux[12] = "";
	if(token_id){	
		for(int i = 0; i < tabla->tupla_cnt; i++){
			if(tabla->atributos[i]->id == token_id){
				snprintf(aux,sizeof(aux),"%s%u",tabla->atributos[i]->tipo,tabla->atributos[i]->id);
				strcat(salida,aux);
			}
		}
		char straux[3] = "";
		snprintf(straux,sizeof(straux),"%c",TILDE);
		strcat(salida,straux); 
	}
	else {
		char straux[8] = "";
		if(delim != TILDE) {
			snprintf(straux,sizeof(straux),"%s%c",convertir_delim(delim),TILDE);
			strcat(salida,straux);
		}
	}
	if(strlen(aux) > 0){
		if(strcmp(get_token_type(aux),"OP") == 0){
			char straux[8] = "";
			if(strchr(DELIM,delim) != NULL && delim != TILDE) {
				snprintf(straux,sizeof(straux),"%s%c",convertir_delim(delim),TILDE);
				strcat(salida,straux);
			}
		}
	}
	return;
}

char *convertir_delim(char delim){
	switch(delim){
		case '(': return "PO"; //parenthesis opens
		case ')': return "PC"; //par... closes
		case '{': return "BO"; //bracket opens
		case '}': return "BC"; //br... closes
		case ';': return "SC"; //semicolon
		case ',': return "CO"; //comma
		default: return "ERD";
	}
}

//se leen los tokens hasta formar sentencias y se evalua que sean sintácticamente correctas
char *parsear_tokens(char *tokens, struct Tabla_s *tabla){
	char copia[SHRT_MAX] = "";
	strcpy(copia, tokens);
	char buffer[240][6];
	int sentence_ctr = 0;
	int i = 0;
	char* context = NULL;
	char* token = strtok_r(copia, "`", &context);
	while (token != NULL){
		strcpy(buffer[i],token);
		if(strcmp(token,"SC") == 0) {
			if(eval_sentencia(buffer, i+1,&T_GLOBAL)){
				printf("\nsentencia %i valida", ++sentence_ctr);
				i = -1;
			}
			else {
				printf("\nError de sintaxis: sentencia %i no valida\n", ++sentence_ctr);
				return NULL;
			}	
		}
    	token = strtok_r(NULL, "`", &context);
    	i++;
	}
	free(tokens);
	return NULL; //?
}

//se evalua que tipo de sentencia es y se escoge la regla sintáctica que se va a evaluar
int eval_sentencia(char buffer[][6], int buffer_size, struct Tabla_s *tabla){ 
	//TODO: FUNCIONES, EXPRESIONES, CONDICIONALES, CICLOS, SWITCH, SENTENCIAS
	
	//solo se encuentran implementadas asignaciones y declaraciones
	return eval_dec_as(buffer, buffer_size,tabla);
}

//automata para evaluar cadenas de asignación/declaración de variables
int eval_dec_as(char buffer[][6], int buffer_size, struct Tabla_s *tabla){
	//printf("abuba");
	int status = 1;
	int id = -1;
	char var_type[5] = "";
	char var_type_aux[5] = "";
	int var_id = 0;
	int token_type = -1;
	int initial = 0;
	int is_expr = false;
	char expr_buffer[CHAR_MAX][5];
	int expr_ptr = 0;
	
	for(int i = 0; i < buffer_size; i++){  
		id = -1;
		
		token_type = eval_token_type(buffer[i]);
		if(status == 1 && token_type == 2){
			id = get_token_id(buffer[i]);
			if(strcmp(tabla->atributos[id-1]->t_dato,"") == 0){
				printf("\nERROR: Variable %s no inicializada", 
					tabla->atributos[id-1]->valor); //evaluación semántica
				break;
			}
			var_id = id;
			status = 2;
			initial = 2;
			
		}
		else if(status == 1 && token_type == 3){
			strcpy(var_type,get_token_value(buffer[i], tabla));
			status = 3;
			initial = 3;
			
		} 
		else if(status == 3 && token_type == 1){
			id = get_token_id(buffer[i]);
			if(strcmp(tabla->atributos[id-1]->valor,"TYP") != 0){
				printf("\nERROR: Operador %s no valido en este contexto", 
					tabla->atributos[id-1]->valor);
				break;
			}
			status = 4;
			
		}
		else if(status == 4 && token_type == 2){
  			id = get_token_id(buffer[i]);
  			var_id = id;
  			//actualizar variable con tipo de dato 
  			strcpy(tabla->atributos[id-1]->t_dato,var_type); 
			status = 5;
			
		}
		else if((status == 2 || status == 5) && token_type == 1){
			id = get_token_id(buffer[i]);
			if(strcmp(tabla->atributos[id-1]->valor,"AS") != 0){
				printf("\nERROR: Operador %s no valido en este contexto", 
					tabla->atributos[id-1]->valor);
				break;
			}
			status = 6;
			
		}
		else if(status == 6 && (token_type == 4 || token_type == 2)){
			//se compara el tipo de dato del token y de la variable en la sentencia
			id = get_token_id(buffer[i]);
			if(token_type == 2){
				if(strcmp(tabla->atributos[id-1]->t_dato,"") == 0){
					printf("\nERROR: Variable %s no inicializada", 
						tabla->atributos[id-1]->valor); //evaluación semántica
					break;
				}
				strcpy(var_type_aux,tabla->atributos[id-1]->t_dato);
				if(strcmp(tabla->atributos[var_id-1]->t_dato,var_type_aux) != 0){
					printf("\nERROR: Inconsistencia en tipos de dato: %s y %s", 
						tabla->atributos[var_id-1]->t_dato,var_type_aux);
					break;
				}
			} else{
				char const_type[10] = "";
				strcpy(const_type,get_token_type(buffer[i]));
				if(strcmp(tabla->atributos[var_id-1]->t_dato,const_type) != 0){		
					printf("\nERROR: Inconsistencia en tipos de dato: %s y %s", 
						tabla->atributos[var_id-1]->t_dato,const_type);
					break;
				}	
			}
			//se guarda id de la constante o id en la entrada de la variable.
			tabla->atributos[var_id-1]->valor_var = id; 
			strcpy(expr_buffer[expr_ptr++],buffer[i]);
			status = 7;
			
		}
		else if(status == 6 && token_type == 5){
			strcpy(expr_buffer[expr_ptr++],buffer[i]);
			is_expr = true;
			
		}
		else if(status == 7 && token_type == 5){
			strcpy(expr_buffer[expr_ptr++],buffer[i]);
			is_expr = true;
			
		}
		else if(status == 7 && token_type == 7){
			if(is_expr) {
				char *expr = eval_expr(expr_buffer, expr_ptr, tabla);
				if(expr == NULL) return false;
				strcpy(tabla->atributos[var_id-1]->expr,expr);
				tabla->atributos[var_id-1]->valor_var = 0;
				free(expr);
				is_expr=false;
			} 
			if(initial == 3) status = 4;
			else status = 1;
			
		}
		else if(status == 7 && token_type == 1){
			is_expr = true;
			strcpy(expr_buffer[expr_ptr++],buffer[i]);
			status = 9;
			
		}
		else if(status == 9 && token_type == 5) {
			strcpy(expr_buffer[expr_ptr++],buffer[i]);
		}
		else if(status == 9 && (token_type == 4 || token_type == 2)){
			id = get_token_id(buffer[i]);
			if(token_type == 2){
				if(strcmp(tabla->atributos[id-1]->t_dato,"") == 0){
					printf("\nERROR: Variable %s no inicializada", 
						tabla->atributos[id-1]->valor); //evaluación semántica
					break;
				}
				strcpy(var_type_aux,tabla->atributos[id-1]->t_dato);
				if(strcmp(tabla->atributos[var_id-1]->t_dato,var_type_aux) != 0){
					printf("\nERROR: Inconsistencia en tipos de dato: %s y %s", 
						tabla->atributos[var_id-1]->t_dato,var_type_aux);
					break;
				}
			} else{
				char const_type[10] = "";
				strcpy(const_type,get_token_type(buffer[i]));
				if(strcmp(tabla->atributos[var_id-1]->t_dato,const_type) != 0){		
					printf("\nERROR: Inconsistencia en tipos de dato: %s y %s", 
						tabla->atributos[var_id-1]->t_dato,const_type);
					break;
				}	
			}
			strcpy(expr_buffer[expr_ptr++],buffer[i]);
			status = 7;
			
		}
		else if((status == 5 || status == 7) && token_type == 6) {
			if(is_expr){
				char *expr = eval_expr(expr_buffer, expr_ptr, tabla);
				if(expr == NULL) return false;
				strcpy(tabla->atributos[var_id-1]->expr,expr);
				tabla->atributos[var_id-1]->valor_var = 0;
				free(expr);
				is_expr = false;
			}
			status = 8;
		}
		else break;
	}
	if(status == 8) return true;
	else return false;
}

//verifica el tipo de token que es y retorna un valor entero segun la clasificación
int eval_token_type(char *buffer){
	
	char ty_buffer[3] = "";
	strcpy(ty_buffer,get_token_type(buffer));
	
	if(strcmp(ty_buffer,"OP") == 0) return 1;
	else if(strcmp(ty_buffer,"ID") == 0) return 2;
	else if(strcmp(ty_buffer,"KW") == 0) return 3;
	else if(strcmp(ty_buffer,"IN") == 0 || strcmp(ty_buffer,"FL") == 0 
		|| strcmp(ty_buffer,"CH") == 0 || strcmp(ty_buffer,"ST") == 0) return 4;
	else if(strcmp(ty_buffer,"PO") == 0 || strcmp(ty_buffer,"PC") == 0) return 5;
	else if(strcmp(ty_buffer,"SC") == 0) return 6;
	else if(strcmp(ty_buffer,"CO") == 0) return 7;
	else return 0;
}

//retorna los dos primeros caracteres del token que identifican su tipo
char *get_token_type(char *buffer){
	
	char *ty_buffer = malloc(4*sizeof(char));
	
	int ty_buffer_ptr = 0;
	char *end;
	for(int i = 0; i < strlen(buffer); i++){
		if(isalpha(buffer[i])) ty_buffer[ty_buffer_ptr++] = buffer[i];
	}
	ty_buffer[ty_buffer_ptr] = '\0';
	return ty_buffer;
}

//funcion util para identificar el tipo de dato al que se refiere una keyword
char *get_token_value(char *buffer, struct Tabla_s *tabla){
	int id = get_token_id(buffer);
	char *str_aux = malloc(4 * sizeof(char));
	for(int i = 0; i < 2; i++){
		str_aux[i] = tabla->atributos[id-1]->valor[i];
	}
	str_aux[2] = '\0';
	for(int i = 0; i < strlen(str_aux); i++){
		str_aux[i] = toupper(str_aux[i]);
	}
	return str_aux;
}

//retorna los caracteres numéricos del token, que hacen referencia a su ID en la tabla
int get_token_id(char *buffer){
	char *id_buffer = malloc(5 * sizeof(char));
	int id_buffer_ptr = 0;
	char *end;
	for(int i = 0; i < strlen(buffer); i++){
		if(isdigit(buffer[i])) id_buffer[id_buffer_ptr++] = buffer[i];
	}
	id_buffer[id_buffer_ptr] = '\0';
	int aux = (int)strtol(id_buffer, &end, 10); 
	free(id_buffer);
	return aux;
}

// lee la expresión y la convierte a postorden, la almacena en una cadena separada por espacios.
char *eval_expr(char buffer[CHAR_MAX][5], int buffer_ptr, struct Tabla_s *tabla){ 
	char *salida = malloc(254 * sizeof(char));
	salida[0] = '\0';
	struct Stack *pila = createStack();
	for(int i = 0; i < buffer_ptr; i++){
		//printf("%s ", buffer[i]);
 		int token_type = eval_token_type(buffer[i]);
		switch(token_type){
			case 1:
				if( isEmpty(pila) || strcmp(peek(pila),"PO") == 0 || 
					eval_pres(buffer[i],tabla) > eval_pres(peek(pila),tabla)){
					push(pila,buffer[i]);
					break;
				}
				if(strcmp(buffer[i], peek(pila)) == 0){
					if(strcmp("EXP", get_token_value(buffer[i], tabla)) == 0){
						push(pila,buffer[i]);
						break;
					} 
					else {
						strcat(salida, pop(pila));
						strcat(salida, " ");
					}
				}
				while(!isEmpty(pila) && strcmp(peek(pila),"PO") != 0 && 
					eval_pres(peek(pila),tabla) >= eval_pres(buffer[i],tabla)){
					strcat(salida, pop(pila));
					strcat(salida, " ");
				}
				push(pila, buffer[i]);
				break;
			case 2:
			case 4:
				strcat(salida,buffer[i]);  
				strcat(salida," "); //:p
				break;
			case 5:
				if(strcmp(buffer[i],"PO") == 0) push(pila,"PO");
				else{
					while(true){
						char aux[5] = "";
						strcpy(aux,pop(pila));
						if(strcmp(aux,"PO") == 0) break;
						else {
							strcat(salida,aux);
							strcat(salida, " ");
						}
						if(isEmpty(pila)){
							printf("\nERROR: Parentesis disparejos en la expresion, falta '('");
							return NULL;				
						}
					}
				}
				break;
			default: break;//error
		}
	}
	while(!isEmpty(pila)){
		char aux[5] = "";
		strcpy(aux,pop(pila));
		if(strcmp(aux,"PO") == 0) {
			printf("\nERROR: Parentesis disparejos en la expresion, falta ')'");
			return NULL;
		}
		else {
			strcat(salida,aux);
			strcat(salida, " ");
		}
	}
	free(pila);
	char *aux = procesar_expr(salida,tabla);
	free(salida);
	return aux;
}

//recibe un token de operador y retorna un entero según su nivel de presedencia
//incluye todos los operadores reconocidos
int eval_pres(char *buffer, struct Tabla_s *tabla){
	char type[4] = "";
	strcpy(type,get_token_value(buffer,tabla));
	if(strcmp("NOT",type) == 0) return 10; // '-' unario
	else if(strcmp("EXP",type) == 0) return 9;
	else if(strstr("MULDIVMOD",type) != NULL) return 8;
	else if(strstr("ADDSUS",type) != NULL) return 7;
	else if(strstr("RIRE",type) != NULL) return 6;
	else if(strstr("LTMTMELE",type) != NULL) return 5;
	else if(strstr("EQUNEQ",type) != NULL) return 4;
	else if(strcmp("AND",type) == 0) return 3;
	else if(strcmp("OR",type) == 0) return 2;
	else if(strcmp("AS",type) == 0) return 1;
}

//convierte la cadena a preorden sustituye el token de op por su valor
char *procesar_expr(char *expr, struct Tabla_s *tabla){
	char *output = malloc(strlen(expr) * 2* sizeof(char));
	output[0] = '\0';
	struct Stack *pila = createStack();
	char* context = NULL;
	char* token = strtok(expr, " ");
	while (token != NULL){
		//printf("\n%s",token);
		if(eval_token_type(token) == 4 || eval_token_type(token) == 2) push(pila,token);
		else{
			char aux[70] = ""; 
			char aux1[30] = "";
			char aux2[30] = "";
			strcpy(aux1,pop(pila));
			strcpy(aux2,pop(pila));
			snprintf(aux,sizeof(aux),"%s %s %s", get_token_value(token, tabla), aux2, aux1);
			push(pila,aux);
		}
    	token = strtok(NULL, " ");
	}
	strcpy(output,pop(pila));
	free(pila);
	return output; 
}

