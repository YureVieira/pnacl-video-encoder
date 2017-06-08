/*
 * WebmMuxer.h
 *
 *  Created on: 24 de nov de 2015
 *  Author: Joaquim neto@LME (www.github.com/joaquimmnetto)
 */

#ifndef WEBM_MUXER_H_
#define WEBM_MUXER_H_

#include <string>
#include <deque>

#include <ppapi/cpp/instance.h>
#include <ppapi/utility/threading/simple_thread.h>

#include "libwebm/mkvmuxer.hpp"

#include "libwebm/mkvwriter.hpp"
#include "tipos.h"
/*
 * Classe que usa a libwebm para compilar frames em formato VP8/VP9 em um arquivo .webm
 *
 */
class WebmMuxer {
public:

	/**Cria uma nova instância
	 *
	 * @param instance - Instância atual do pepper.
	 *
	*/
	WebmMuxer( pp::Instance& instance );
	/**
	 * Passa parâmetros necessários para o início da gravação do arquivo de vídeo.
	 * Deve ser chamado antes de se adicionar frames.
	 *
	 * @param video_width - Largura, em pixels, do vídeo de saída.
	 * @param video_height - Altura, em pixels, do vídeo de saída.
	 *
	 */
	void ConfigureVideo( int video_width, int video_height );
	/**
	 * Adiciona um frame de vídeo ao arquivo. O frame adicionado deve ser sempre de timestamp maior do que o anterior.
	 *
	 * @param data - uint8* contendo os dados do frame.
	 * @param length - tamanho de data.
	 * @param timestamp - timestamp do frame. Deve sempre crescer monotonicamente.
	 * @param key_frame - boolean indicando se este é um key_frame, ou não.
	 *
	 *
	 * @return true, se a operação teve sucesso, false caso contrário.
	 *
	 */
	bool AddVideoFrame( byte* data, uint32 length, uint64 timestamp, bool key_frame );

	/**
	 * Encerra a gravação, finalizando o arquivo, e gravando frames restantes.
	 */
	bool Finish();

	inline void SetFileName( const std::string _file_name ) {file_name = _file_name;}

	virtual ~WebmMuxer();
private:
	/**Instância do pepper*/
	pp::Instance& instance;
	/**Nome do arquivo que será salvo*/
	std::string file_name;
	/**Objeto Segment da libwebm. Ele representa o video que está sendo salvo.*/
	mkvmuxer::Segment* pSegment;
	/**Objeto MkvWriter da libwebm. É passado para o segment, e é o escritor que será usado para salvar o arquivo em disco.
	 * Em caso de necessidade de maneiras diferentes de escrita, pode se implementar a interface IMkvWriter e usar o objeto criado para substituir este.*/
	mkvmuxer::MkvWriter writer;
	/**Último frame a ser salvo com sucesso*/
	uint64 last_frame_ts;
	/**Número de frames que não foram salvos em um intervalo. Necessário para se congelar a imagem quando se perde muitos frames.*/
	int delayed_frame_count;

	/**Número da trilha de vídeo criada por pSegment.*/
	int video_track_num;

	/**Largura do vídeo a ser salvo.*/
	int video_width;
	/**Altura do vídeo a ser salvo*/
	int video_height;

	/**Flag indicando se o muxer já foi inicializado*/
	bool initialized;
	/**Flag indicando se o muxer já foi finalizado*/
	bool finished;

	/**Inicializa os objetos da classe, e deixa a mesma pronta para a gravação
	 * @param file_name - string com o caminho+nome do arquivo a ser salvo.
	 * */
	int Init( std::string file_name );
	/***
	 * Cria um objeto Frame da libwebm.
	 *
	 * @param data - uint8* contendo os dados do frame.
	 * @param length - tamanho de data.
	 * @param timestamp - timestamp do frame. Deve sempre crescer monotonicamente.
	 * @param track - Qual a track a qual esse frame irá pertencer
	 * @param key_frame - boolean indicando se este é um key_frame, ou não.
	 *
	 * @param frame[out] - Referência a um objeto frame vazio. É o retorno do método.
	 */
	void CreateFrame( byte* data, uint32 length, uint64 timestamp, int track, bool key_frame, /*out*/mkvmuxer::Frame& frame );
	/**
	 * Produz o efeito de imagem congelada quando se perde frames.
	 *
	 * @param current_ts - timestamp na hora do 'descongelamento'.
	 *
	 */
	void SaveDelayed(uint64 current_ts);



};

#endif /* WEBM_MUXER_H_ */
