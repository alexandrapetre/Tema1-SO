#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE     255
#define VECTORSIZE 16
#define delim "\t[]<>-=*/%!&|^.,:;()\\ \n"
#define delimFile "\" "
#define delim1 "\\\n "

	struct sIndex{

		char *value;
		char *key;
		struct sIndex *next;  

	};

	struct sIndex **hashMap;

	int hashFunction(char *);
	void removeKey(char *);
	void insert(char *, char *);
	void printList(struct sIndex *);
	void addValue(struct sIndex **, char *, char *);
	void destroy(struct sIndex **);
	void parsare(char *, FILE *, int *, int *, char **, int, int *);
	char *get(char *);
	int isKey(char *);

/*init function */
void *initFunction(size_t size){
	void *mem = malloc(size);

	if(mem == NULL)
		exit(1);

	return mem;
}

/* hash function */
int hashFunction (char *str){
	int hash = 0;
	int c;

	c = str[0];
	hash = c % 16;

	return hash;
}

/* removes entry from HashMap by key*/
void removeKey(char *key){
	int hash = hashFunction(key);
	struct sIndex *list = hashMap[hash];
	struct sIndex *prev;

	if((list != NULL) && (strcmp(list->key, key) == 0)){
		hashMap[hash] = list->next;
		free(list->key);
		free(list->value);
		free(list);
		return; 
	}

	while(list != NULL && (strcmp(list->key, key) != 0)){
		prev = list;
		list = list->next;
	}

	if(list == NULL)
		return;

	prev->next = list->next;
	free(list->key);
	free(list->value);
	free(list);
}

/* prints linkedList */
void printList(struct sIndex *currentIndex){

	while(currentIndex != NULL){
		printf("  (%s, %s) \n", currentIndex->key, 
			currentIndex->value);            
		currentIndex  = currentIndex->next;
	}
}

/*adds at the end of the list */
void addValue(struct sIndex **first, char *key, char *value){
	struct sIndex *last = *first;
	struct sIndex *elem = initFunction(sizeof(*elem));

	elem->value = malloc((strlen(value) + 20) * sizeof(char));
	if(elem->value == NULL)
		exit(1);

	memcpy(elem->value, value, strlen(value) + 1);
	elem->key = malloc((strlen(key) + 20) * sizeof(char));
	if(elem->key == NULL)
		exit(1);
	memcpy(elem->key, key, strlen(key) + 1);
	elem->next = NULL;

	if(*first == NULL){
		*first = elem;
		return;
	}

	if(strcmp(last->key, key) == 0){
		removeKey(key);
		insert(key, value);
		return;
	}

	while(last->next != NULL){
		if(strcmp(last->key, key) == 0)
			removeKey(key);
		last = last->next;
	}
	last->next = elem;
}

/* inserts (key, value) in HashMap */
void insert(char *key, char *value){
	int hash = hashFunction(key);

	addValue(&hashMap[hash], key, value);
}

/* gets value for a key */
char *get(char *key){
	int hash = hashFunction(key);
	struct sIndex *list = hashMap[hash];
	char *value = NULL;

	while(list != NULL){
		if(strcmp(list->key, key) == 0)
			return list->value;
		list = list->next;
	}
	return value;
}

/* check if key exists in HashMap */
int isKey(char *key){
	int hash = hashFunction(key);
	struct sIndex *list = hashMap[hash];

	while(list != NULL){
		if(strcmp(list->key, key) == 0)
			return 1;
		list = list->next;
	}
	return 0;
}

/* destroy the HashMap */
void destroy(struct sIndex **currentIndex){
	struct sIndex *current = *currentIndex;
	struct sIndex *tmp;

	while(current != NULL){
		tmp = current;
		current = current->next;
		free(tmp->key);
		free(tmp->value);
		free(tmp);
	}
	*currentIndex  = NULL;
}

/* count tokens */
int countTokens(char *buffer){
	int count = 0;
	char *token = strtok(buffer, " \n");

	while( token != NULL){
		count++;
		token = strtok(NULL, " \n");
	}
	return count;
}

/* adds new element into hashMap*/
void createNewVariable(int numberOfTokens, char *token, char *key){
	char *val = NULL;
	char *valueConcat = malloc(50 * sizeof(char));

	if(valueConcat == NULL)
		exit(1);

	valueConcat[0] = '\0';

	while(numberOfTokens != 0){
		token = strtok(NULL, " \n");
		val = get(token);
		if(val != NULL){
			strncat(valueConcat, val, strlen(val) + 1);
			if(numberOfTokens > 1)
				strncat(valueConcat, " ", 2);
		}else{
			strncat(valueConcat, token, strlen(token) + 1);
			if(numberOfTokens > 1)
				strncat(valueConcat, " ", 2);
		}
		numberOfTokens--;
	}
	insert(key, valueConcat);
	free(valueConcat);
}

void concatVariable(int *varDef, char *copy){
	struct sIndex *list = hashMap[*varDef];
	char *newValue = malloc(30 * sizeof(char));
	int lenBuffer = strlen(copy);
	int i = 0;
	int ok = 0;
	char c[2];

	if(newValue == NULL)
		exit(1);

	newValue[0] = '\0';

	memcpy(newValue, list->value, strlen(list->value) + 1);

	for(i = 0; i < lenBuffer; i++){
		if((strchr(delim1, copy[i])) == NULL){
			c[0] = copy[i];
			c[1] = '\0';
			strncat(newValue, c, 3);
			strncat(newValue, " ", 2);
		}
		if(copy[i] == '\\')
			ok = 1;
	}

	if(ok == 0){
		newValue[strlen(newValue) - 1] = '\0';
		*varDef = -1;
	}

	memcpy(list->value, newValue, strlen(newValue) + 1);
	free(newValue);
}

/*replace with values from hashMap */
void replaceToken(char *copy, char *newBuffer){
	int i, lenBuffer = strlen(copy);
	char c[2];
	char *value = NULL;
	int quotes = 0;
	char *element = malloc ((strlen(copy) + 10) * sizeof(char));
	
	if(element == NULL)
		exit(1);

	newBuffer[0] = '\0';

	element[0] = '\0';

	for(i = 0; i < lenBuffer; i++){
		if((strchr(delim, copy[i]) != NULL)){
			if(copy[i] == '"' && quotes != 1)
				quotes = 1;
			else
				quotes = 0;

			c[0] = copy[i];
			c[1] = '\0';
			if(element == NULL)
				strncat(newBuffer, c, 3);
			else{
				value = get(element);
				if(value == NULL){
					strncat(newBuffer, element,
						strlen(element) + 10);
					strncat(newBuffer, c, 3);
					element[0] = '\0';
				}else{
					if(quotes == 0){
						strncat(newBuffer, value,
							strlen(value) + 10);
						strncat(newBuffer, c, 3);
						element[0] = '\0';
					}else{
						strncat(newBuffer, element,
							strlen(element) + 10);
						strncat(newBuffer, c, 3);
						element[0] = '\0';
					}
					
				}
			}
		}else{
			c[0] = copy[i];
			c[1] = '\0';
			strncat(element, c,3);
		}
	}

	free(element);
	free(value);
}

/* send to parse files .h */
void openFile(char *buffer, char *fileName, FILE *fileOUT, 
	char **fileDirectories, int directory, int *index,
	int *varDef){
	FILE *fileIN = NULL;
	int printNext = 0, defineNext = 0;

	if(fileName != NULL)
		fileIN = fopen(fileName, "r");
	
	(*index)++;

	if(fileIN == NULL && directory == *index)
		if(fileIN == NULL)
			exit(1);
	if(fileIN == NULL)
		return;

	while((fgets(buffer, BUFSIZE, fileIN)) != NULL)
		parsare(buffer, fileOUT, &printNext, &defineNext, 
			fileDirectories, directory, varDef);

	if(fileIN != NULL)
		fclose(fileIN);
}

/* parses the lines strarting with # */
void parseDefine(char *token, int *numberOfTokens, int *changed,
	int *varDef){
	char *key = NULL, *value = NULL;
	char c[2];

	if(*numberOfTokens == 3){
		token = strtok(NULL, " \n");
		key = malloc((strlen(token) + 1)  * sizeof(char));
		if(key == NULL)
			exit(1);

		memcpy(key, token, strlen(token) + 1);
		token = strtok(NULL, " \n");
		value = malloc((strlen(token) + 1) * sizeof(char));
		if(value == NULL)
			exit(1);

		memcpy(value, token, strlen(token) + 1);
		if(isKey(key) == 0)
			insert(key,value);
		*changed = 1;
		free(key);
		free(value);
	}else if(*numberOfTokens == 2){/* define with no value */
		token = strtok(NULL, " \n");
		key = malloc((strlen(token) + 1) * sizeof(char));
		if(key == NULL)
			exit(1);

		memcpy(key, token, strlen(token) + 1);
		value = "";
		insert(key,value);
		*changed = 1;
		free(key);
	}else if(*numberOfTokens == 4){
		token = strtok(NULL, " \n");
		key = malloc((strlen(token) + 1)  * sizeof(char));
		if(key == NULL)
			exit(1);

		memcpy(key, token, strlen(token) + 1);

		*varDef = hashFunction(key);
		token = strtok(NULL, " \n");
		value = malloc((strlen(token) + 8) * sizeof(char));
		if(value == NULL)
			exit(1);

		memcpy(value, token, strlen(token) + 1);
		c[0] = ' ';
		c[1] = '\0';
		strncat(value, c, 3);

		if(isKey(key) == 0)
			insert(key,value);
		*changed = 1;
		free(key);
		free(value);
	}else if (*numberOfTokens > 4){/* multiple elements define */
		token = strtok(NULL, " \n");
		key = malloc((strlen(token) + 1)* sizeof(char));

		if(key == NULL)
			exit(1);

		memcpy(key, token, strlen(token) + 1);
		*numberOfTokens -= 2;
		createNewVariable(*numberOfTokens, token, key);
		free(key);
		*changed = 1;
	}
}

/* parses the lines which start with #undef */
void parseUnDef(char *token, int *changed){
	char *key = NULL;

	token = strtok(NULL, " \n");
	key = malloc((strlen(token) + 1) * sizeof(char));
	if(key == NULL) 
		exit(1);
	key[0] = '\0';
	memcpy(key, token, strlen(token) + 1);
	removeKey(key);
	free(key);
	*changed = 1;
}

/* if condition */
void ifFunction(int *printNext, char *key){
	char *value = NULL;

	if((strcmp(key, "0") != 0) && (strlen(key) == 1)){
		*printNext = 1;
	}else{
		if(strcmp(key, "0") == 0){
			*printNext = -1;
		}else{
			value = get(key);
			if(strcmp(value, "0") == 0)
				*printNext = -1;
			else
				*printNext = 1;
		}
	}
}

/* parses the lines with if condition */
void parseIf(char *token, int *printNext, int *changed){
	char *key = NULL;

	key = malloc((strlen(token) + 1) * sizeof(char));
	if(key == NULL)
		exit(1);

	memcpy(key, token, strlen(token) + 1);
	ifFunction(printNext, key);
	*changed = 1;
	free(key);
}

/* parses the lines with else condition */
void parseElse(int *printNext, int *changed){
	if(*printNext == 2 || *printNext == -1)
		*printNext = 1;
	else
		*printNext = 2;
	
	*changed = 1;
}

/* parses the lines with elif condition */
void parseElif(char *token, int *printNext, int *changed){
	char *key = NULL;

	token = strtok(NULL, delim);
	key = malloc((strlen(token) + 1) * sizeof(char));

	if(key == NULL)
		exit(1);

	memcpy(key, token, strlen(token) + 1);

	if(*printNext == 2 || *printNext == -1)
		ifFunction(printNext, key);

	*changed = 1;
	free(key);
}

/* parses the lines with endif condition */
void parseEndIf(int *printNext, int *defineNext, int *changed){
	*changed = 1;

	if(*printNext != 0)
		*printNext = 0;

	if(*defineNext != 0)
		*defineNext = 0;
}

/* parses the lines with #ifdef*/
void parseIfDef(char *token, int *defineNext, int *changed){

	token = strtok(NULL, delim);
	if(isKey(token))
		*defineNext = 1;
	else
		*defineNext = -1;
	*changed = 1;
}

void parseIfNDef(char *token, int *defineNext, int *changed){
	
	token = strtok(NULL, delim);
	if(isKey(token) == 0)
		*defineNext = 1;
	else
		*defineNext = -1;
	*changed = 1;
}

/* parses the lines which start with #include */
void parseInclude(char **fileDirectories, char *token, char *copy,
	int *directory, int *parameters, FILE *fileOUT, int *changed,
	int *varDef){
	char *fileName = NULL;
	int i;
	int index = 0;

	for(i = 0; i < *directory; i++){
		*parameters = 1;
		if(fileDirectories[i] != NULL){
			fileName = malloc( 50 * sizeof(char));
			memcpy(fileName, fileDirectories[i], 
				strlen(fileDirectories[i]) + 1);
			if(fileName == NULL)
				exit(1);
			strncat(fileName, "/", 2);
			strncat(fileName, token, strlen(token) + 1);
			openFile(copy, fileName, fileOUT, fileDirectories,
				*directory, &index, varDef);
		}
		free(fileName);
	}

	if(*parameters == 0)
		openFile(copy, token, fileOUT, fileDirectories, *directory,
			&index, varDef);

	*changed = 1;
}

/* parses each line */
void parsare(char *buffer, FILE *fileOUT, int *printNext, 
	int *defineNext, char **fileDirectories, int directory,
	int *varDef){
	char *token = NULL;
	char *copy = NULL, *newBuffer = NULL, *copyBuffer = NULL;
	int parameters = 0, numberOfTokens;
	int changed = 0;

	copy = malloc((strlen(buffer) + 1) *sizeof(char));

	if(copy == NULL)
		exit(1);
	memcpy(copy, buffer, strlen(buffer) + 1);

	copyBuffer = malloc(BUFSIZE * sizeof(char));
	if(copyBuffer == NULL)
		exit(1);
	memcpy(copyBuffer, buffer, strlen(buffer) + 1);

	newBuffer = malloc(BUFSIZE * sizeof(char));
	if(newBuffer == NULL)
		exit(1);

	newBuffer[0] = '\0';
	numberOfTokens = countTokens(copyBuffer);

	token = strtok(buffer, " \n");
	if(*varDef != -1 && numberOfTokens == 2){
		changed = 1;
		concatVariable(varDef, copy);
	}

	while(token != NULL){
		if(strcmp(token, "#define") == 0){
			parseDefine(token, &numberOfTokens, &changed, varDef);
			token = strtok(NULL, " \n");
		}else if(strcmp(token, "#undef") == 0){
			parseUnDef(token, &changed);
			token = strtok(NULL, " \n");
		}else if(strcmp(token, "#if") == 0){
			token = strtok(NULL, delim);
			parseIf(token, printNext, &changed);
		}else if(strcmp(token, "#else") == 0){
			parseElse(printNext, &changed);
			token = strtok(NULL, delim);
		}else if(strcmp(token, "#elif") == 0){
			parseElif(token, printNext, &changed);
			token = strtok(NULL, delim);
		}else if(strcmp(token, "#endif") == 0){
			parseEndIf(printNext, defineNext, &changed);
			token = strtok(NULL, delim);
		}else if(strcmp(token, "#ifdef") == 0){
			parseIfDef(token, defineNext, &changed);
			token = strtok(NULL, delim);
		}else if(strcmp(token, "#ifndef") == 0){
			parseIfNDef(token, defineNext, &changed);
			token = strtok(NULL, delim);
		}else if(strncmp(token, "#include", 8) == 0){
			token = strtok(NULL, delimFile);
			parseInclude(fileDirectories,token, copy,
				&directory, &parameters, fileOUT, &changed,
				varDef);
		}else{
			if(changed == 1){
				if(*printNext == -1)
					*printNext = 2;
				if(*defineNext == -1)
					*defineNext = 2;
				break;
			}

			if((*printNext != 2 && *printNext != -1) && 
				(*defineNext != 2 && *defineNext != -1))
				replaceToken(copy, newBuffer);
			break;
		}
	}

	if(strcmp(newBuffer, "\0") != 0)
		fputs(newBuffer, fileOUT);

	free(newBuffer);
	free(copyBuffer);
	free(copy);
}

void parseArgumentsD(char **arg, int *i){
	char *key = NULL;
	char *value = NULL;

	if(strlen(arg[*i]) == 2){
		key = strtok(arg[(*i) + 1], delim);
		value = strtok(NULL, delim);
		if(value != NULL)
			insert(key,value);
		else{
			value = "";
			insert(key,value);
		}
		(*i)++;
	}else{
		key = strtok(arg[*i] + 2, delim);
		value = strtok(NULL, delim);
		if(value != NULL)
			insert(key,value);
		else{
			value = "";
			insert(key,value);
		}
	}
}

void parseArgumentsO(char **argv, char *filename_out, 
	int *k2, int *i){
	
	filename_out = malloc((strlen(argv[(*i) + 1]) + 1) * 
		sizeof(char));

	if(filename_out == NULL)
			exit(1);
	memcpy(filename_out, argv[(*i)+1], strlen(argv[(*i) + 1] + 1));
	*k2 = 1;
	(*i)++;
}

void parseArgumentsI(char **argv, char **fileDirectories, 
	int *i, int *directory){

	if(strlen(argv[*i]) == 2){
		fileDirectories[*directory]= malloc(
			(strlen(argv[(*i)+1]) + 1) * sizeof(char));
		if(fileDirectories[*directory] == NULL)
			exit(1);
		memcpy(fileDirectories[*directory], argv[(*i)+1], 
			strlen(argv[(*i) + 1]) + 1);
		(*directory)++;
		(*i)++;
	}
}

void initializeFileDirectories(char *filename_in, char **fileDirectories){
	char *token = NULL;
	if(filename_in != NULL){
		fileDirectories[0] = malloc(1000 * sizeof(char));
		if(fileDirectories[0] == NULL)
			exit(1);
		token = strtok(filename_in, delim);
		memcpy(fileDirectories[0], token, strlen(token) + 1);
		strncat(fileDirectories[0], "/", 2);
		token = strtok(NULL, delim);
		
		strncat(fileDirectories[0], token, strlen(token) + 1);
	}
}


int main(int argc, char **argv){
	char buffer[BUFSIZE];
	int k1 = 0, k2 = 0, countFiles = 0;
	int printNext = 0, defineNext = 0, directory = 1;
	int i, rc;
	int varDef = -1;
	char *filename_in = NULL, *filename_out = NULL;
	char **fileDirectories = NULL;
	char firstTwo[2];
	FILE *fileIN = NULL;
	FILE *fileOUT = NULL;

	hashMap = malloc(VECTORSIZE * sizeof(struct sIndex));
	if(hashMap == NULL)
		exit(1);

	fileDirectories = malloc(10 * sizeof(char *));
	if(fileDirectories == NULL)
		exit(1);

	for(i = 0; i < 10; i++) {
		fileDirectories[i] = NULL;
	}

	for(i = 0; i < VECTORSIZE; i++)
		hashMap[i] = NULL;

	for(i = 1; i < argc; i++){
		if(strlen(argv[i]) >= 2){
			strncpy(firstTwo, argv[i], 2);
			firstTwo[2] = '\0';

			if(strncmp(firstTwo, "-D", 2) == 0){
				parseArgumentsD(argv, &i);
			}else if(strncmp(firstTwo, "-o", 2) == 0){
				parseArgumentsO(argv, filename_out, &k2, &i);
			} else if(strncmp(firstTwo, "-I", 2) == 0){
				parseArgumentsI(argv, fileDirectories, &i, 
					&directory);
			}else if(k1 != 1){
					filename_in = malloc((strlen(argv[i]) + 10) *
						sizeof(char));
					if(filename_in == NULL)
						exit(1);
					memcpy(filename_in, argv[i], 
						strlen(argv[i]) + 1);
					k1 = 1;
					countFiles++;
				} else{
					if((k1 == 1) && (countFiles != 2) ){
						filename_out = malloc((strlen(argv[i]) + 10) * 
							sizeof(char));
						if(filename_out == NULL)
							exit(1);
						memcpy(filename_out, argv[i], 
							strlen(argv[i]) + 1);
						k2 = 1;
						countFiles++;
					}else{
						if(countFiles >= 2)
							exit(1);
					}
				}
			}
		}

	/* open source file for reading */
	if(k1 == 1){
		fileIN = fopen(filename_in, "r");
		if(fileIN == NULL)
			exit(1);
	}else{
		fileIN = stdin;
		filename_in = NULL;
	}

	if(k2 == 1){
		fileOUT = fopen(filename_out, "w");
		if(fileOUT == NULL)
			exit(1);
	}else{
		filename_out = NULL;
		fileOUT = stdout;
	}

	initializeFileDirectories(filename_in, fileDirectories);

	/* Read and Write from file */ 
	while((fgets(buffer, BUFSIZE, fileIN)) != NULL)
		parsare(buffer, fileOUT, &printNext, &defineNext, 
			fileDirectories, directory, &varDef);

	for(i = 0; i < VECTORSIZE; i++)
		destroy(&hashMap[i]);

	free(hashMap);

	/* close file */
	if(k1 == 1){
		rc = fclose(fileIN);
		if(rc < 0)
			exit(1);
		free(filename_in);
	}

	if(k2 == 1){
		rc = fclose(fileOUT);
		if(rc < 0)
			exit(1);
		free(filename_out);
	}

	for(i = 0;  i < directory; i++)
		free(fileDirectories[i]);

	free(fileDirectories);
	
	return 0;
}
