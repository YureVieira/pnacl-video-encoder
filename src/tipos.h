/*
 * Defines.h
 *
 *  Created on: 23 de nov de 2015
 *  Author: Joaquim neto@LME (www.github.com/joaquimmnetto)
 */

#ifndef TIPOS_H_
#define TIPOS_H_

//Nomes mais bonitinhos para esses tipos.
typedef int16_t int16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef uint8_t byte;

/**
 * Deleta e nulifica o ponteiro passado. Para ser usado ao invés de operações comuns de delete.
 * @param ptr - Ponteiro a ser deletado e nulificado.
 *
 * @obs feito como macro ao invés de método por maior flexibilidade com tipos.
 */
#define delete_and_nulify(ptr) 	\
	if (ptr) { 					\
		delete ptr; 			\
		ptr = NULL;				\
	}							\


#endif /* TIPOS_H_ */
