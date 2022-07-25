//estructuras y "metodos"

	//entrada en la tabla de simbolos
struct Tupla_tabla {
	int id;
	char tipo[10];
	char valor[SHRT_MAX];
	char t_dato[10]; //funciones y variables
	int valor_var; //variables
	char expr[50]; //usado para almacenar expresiones
};

struct Tupla_tabla *crear_tupla (/*int iden,*/ char type[10], char val[SHRT_MAX]){
	struct Tupla_tabla *nueva = malloc(sizeof(struct Tupla_tabla));
	/*nueva->id = iden;*/
	strcpy(nueva->tipo, type);
	strcpy(nueva->valor, val);
	strcpy(nueva->t_dato, "");
	strcpy(nueva->expr, "");
	nueva->valor_var = 0;
	return nueva;
}

//tabla de simbolos
struct Tabla_s {
	struct Tupla_tabla *atributos[1024];
	int tupla_cnt;
};

/*funcion que agrega tupla a la tabla de simbolos y retorna id de la tupla (evita valores 0)
	verifica también que no se ingrese una tupla con mismo tipo y valor de una existente
*/
int agregar_token (struct Tabla_s *tabla, struct Tupla_tabla *tupla){
	int token_id = 0;
	if(tabla->tupla_cnt > 0){
		for(int i = 0; i < tabla->tupla_cnt; i++){
			if( strcmp(tupla->tipo, tabla->atributos[i]->tipo)==0 && strcmp(tupla->valor, tabla->atributos[i]->valor)==0){
				token_id = tabla->atributos[i]->id;
				return token_id;
			} 
		}
	}
	tupla->id = tabla->tupla_cnt+1;
	tabla->atributos[tabla->tupla_cnt] = tupla;
	token_id = tabla->atributos[tabla->tupla_cnt]->id;
	tabla->tupla_cnt++;
	return token_id;
}

//stack geeks for geeks modificado para trabajar con strings

// Stack type
struct Stack
{
	int top;
	//unsigned capacity;
	char array[30][70];
};

// Stack Operations
struct Stack* createStack( /*unsigned capacity*/ )
{
	struct Stack* stack = (struct Stack*)
		malloc(sizeof(struct Stack));

	if (!stack)
		return NULL;

	stack->top = -1;
	//stack->capacity = capacity;

	//stack->array = (int*) malloc(stack->capacity *sizeof(int));
	//stack->array = (char**) malloc(30 * 4 * sizeof(char));
	return stack;
}
int isEmpty(struct Stack* stack)
{
	return stack->top == -1 ;
}
char *peek(struct Stack* stack)
{
	return stack->array[stack->top];
}
char *pop(struct Stack* stack)
{
	if (!isEmpty(stack))
		return stack->array[stack->top--];
	return "empty";
}
void push(struct Stack* stack, /*char op*/ char *op)
{
	//stack->array[++stack->top] = op;
	strcpy(stack->array[++stack->top], op);
}