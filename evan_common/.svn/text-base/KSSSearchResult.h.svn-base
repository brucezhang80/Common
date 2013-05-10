/*
 * KSSSearchResult.h
 *
 *  Created on: 2010-6-30
 *      Author: evantang
 */

#ifndef KSSSEARCHRESULT_H_
#define KSSSEARCHRESULT_H_

#include "log.h"

#define MAX_TITLE_LEN 128			/*标题最大长度*/
#define MAX_URL_LEN 512			    /*URL最大长度*/
#define MAX_SUMMARY_LEN 256         /*摘要最大长度*/
#define MAX_IMAGEURL_LEN 512        /*image_url的最大长度*/
#define MAX_SITENAME_LEN 512
#define MAX_VIDEOURL_LEN 512
#define MAX_WORD_LENGTH 64

//KssResult结构体 最多为1k
class CKSSResult
{
public:
	CKSSResult()	{}

public:
	/* 将内容填回至buf里，<0表示回填失败,  >0则表示用到的字符空间 */
	int ToCharArray(char* buf, int len)
	{
		if ((unsigned int)len < sizeof(CKSSResult) || len < 0)
		{
			LOG_TXT_ERR("buf len[%d] is not enough for [%d]", len ,sizeof(CKSSResult));
			return -1;
		}

		char * p = buf;
		int p_len = 0;

		/*word_len*/
		unsigned int word_len = strlen(word);
		memcpy(p, &word_len, sizeof(unsigned int));
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);

		/*word*/
		memcpy(p, word, word_len);
		p += word_len;
		p_len += word_len;

		/*docid*/
		memcpy(p, &docid, sizeof(unsigned long long));
		p += sizeof(unsigned long long);
		p_len += sizeof(unsigned long long);

		/*type*/
		memcpy(p, &type, sizeof(int));
		p += sizeof(int);
		p_len += sizeof(int);

		/*title_len*/
		memcpy(p, &title_len, sizeof(int));
		p += sizeof(int);
		p_len += sizeof(int);

		/*title*/
		memcpy(p, title, title_len);
		p += title_len;
		p_len += title_len;

		/*url_len*/
		memcpy(p, &url_len, sizeof(int));
		p += sizeof(int);
		p_len += sizeof(int);

		/*url*/
		memcpy(p, url, url_len);
		p += url_len;
		p_len += url_len;

		/*sitename_len*/
		memcpy(p, &sitename_len, sizeof(int));
		p += sizeof(int);
		p_len += sizeof(int);

		/*sitename*/
		memcpy(p, sitename, sitename_len);
		p += sitename_len;
		p_len += sitename_len;

		/*imageurl_len*/
		memcpy(p, &imageurl_len, sizeof(int));
		p += sizeof(int);
		p_len += sizeof(int);

		/*imageurl*/
		memcpy(p, imageurl, imageurl_len);
		p += imageurl_len;
		p_len += imageurl_len;

		/*image_width*/
		memcpy(p, &img_width, sizeof(int));
		p += sizeof(int);
		p_len += sizeof(int);

		/*image_height*/
		memcpy(p, &img_height, sizeof(int));
		p += sizeof(int);
		p_len += sizeof(int);

		/*summary_len*/
		memcpy(p, &summary_len, sizeof(int));
		p += sizeof(int);
		p_len += sizeof(int);

		/*summary*/
		memcpy(p, summary, summary_len);
		p += summary_len;
		p_len += summary_len;

		/*srctimestamp*/
		memcpy(p, &src_timestamp, sizeof(int));
		p += sizeof(int);
		p_len += sizeof(int);

		/*intimestamp*/
		memcpy(p, &in_timestamp, sizeof(int));
		p += sizeof(int);
		p_len += sizeof(int);

		/*rank*/
		memcpy(p, &rank, sizeof(int));
		p += sizeof(int);
		p_len += sizeof(int);

		/*video_len*/
		memcpy(p, &videourl_len, sizeof(int));
		p += sizeof(int);
		p_len += sizeof(int);

		/*video*/
		memcpy(p, videourl, videourl_len);
		p += videourl_len;
		p_len += videourl_len;

		if (p_len > len)
		{
			LOG_TXT_ERR("buf len[%d] is not enough for [%d] when initi finished.", len , p_len);
			return -1;
		}

		return p_len;
	}

	void Print()
	{
		LOG_TXT_DEBUG("-------------------------------------------------------------------------");
		LOG_TXT_DEBUG("docid: %llu", docid);
		LOG_TXT_DEBUG("type: %d", type);
		LOG_TXT_DEBUG("title_len: %d", title_len);
		LOG_TXT_DEBUG("title: %s", title);
		LOG_TXT_DEBUG("url_len: %d", url_len);
		LOG_TXT_DEBUG("url: %s", url);
		LOG_TXT_DEBUG("sitename_len: %d", sitename_len);
		LOG_TXT_DEBUG("sitename: %s", sitename);
		LOG_TXT_DEBUG("imageurl_len: %d", imageurl_len);
		LOG_TXT_DEBUG("imageurl: %s", imageurl);
		LOG_TXT_DEBUG("summary_len: %d", summary_len);
		LOG_TXT_DEBUG("summary: %s", summary);
		LOG_TXT_DEBUG("src_timestamp: %d", src_timestamp);
		LOG_TXT_DEBUG("in_timestamp: %d", in_timestamp);
		LOG_TXT_DEBUG("rank: %d", rank);
		LOG_TXT_DEBUG("videourl_len: %d", videourl_len);
		LOG_TXT_DEBUG("videourl: %s", videourl);
		LOG_TXT_DEBUG("-------------------------------------------------------------------------");
	}

public:
	/* 初始化，<0表明初始化失败，不是一个正确的result格式, >0则表示用到的字符空间*/
	/*-1 标题过长
	 *-2 url过长
	 *-3 sitename过长
	 *-4 imageurl过长
	 *-5 summary过长
	 *-6 word过长
	 */
	int Initi(const char* buf, const int len, const char* _word)
	{
		/*拷贝词*/
		int word_len = strlen(_word);
		if (word_len > MAX_WORD_LENGTH)
		{
			printf("word too long.");
			return -6;
		}

		strncpy(word, _word, MAX_WORD_LENGTH);
		//printf("word %s\n", word);

		const char *p = buf;
		int p_len = 0;

		//docid
		docid = *(unsigned long long*)p;
		p += sizeof(unsigned long long);
		p_len += sizeof(unsigned long long);
		//pintf("docid %llu\n", docid);

		//type
		type = *(int*)p;
		p += sizeof(int);
		p_len += sizeof(int);
		//printf("type %d\n", type);

		//titleLen
		title_len = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);

		if (title_len > MAX_TITLE_LEN)
		{
			printf("title[%d] too long.", title_len);
			return -1;
		}

		//title
		memcpy(title, p, title_len);
		p += title_len;
		p_len += title_len;

		//printf("title %d %s\n", title_len, title);

		//url_len
		url_len = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);

		if (url_len > MAX_URL_LEN)
		{
			printf("url[%d] too long.", url_len);
			return -2;
		}

		//url
		memcpy(url, p, url_len);
		p += url_len;
		p_len += url_len;

		//printf("url %d %s\n", url_len, url);

		//sitename_len
		sitename_len = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);

		if (sitename_len > MAX_SITENAME_LEN)
		{
			printf("sitename[%d] too long.", sitename_len);
			return -3;
		}

		//sitename
		memcpy(sitename, p, sitename_len);
		p += sitename_len;
		p_len += sitename_len;

		//printf("sitename %d %s\n", sitename_len, sitename);

		//imageurl_len
		imageurl_len = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);

		if (imageurl_len  > MAX_IMAGEURL_LEN)
		{
			printf("imageurl[%d] too long.", imageurl_len);
			return -4;
		}

		//imageurl
		memcpy(imageurl, p, imageurl_len);
		p += imageurl_len;
		p_len += imageurl_len;

		//img_width
		img_width = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);

		//img_height
		img_height = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);

		//printf("image %d %s %d %d\n", imageurl_len, imageurl, img_width, img_height);

		//summary_len
		summary_len = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);

		if (summary_len  > MAX_SUMMARY_LEN)
		{
			printf("summary[%d] too long.", summary_len);
			return -5;
		}

		//summary
		memcpy(summary, p, summary_len);
		p += summary_len;
		p_len += summary_len;

		//printf("summary %d %s\n", summary_len, summary);

		//src_timestamp
		src_timestamp = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);

		//printf("src_timestamp %d\n", src_timestamp);

		//in_timestamp
		in_timestamp = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);
		//printf("%d\n", in_timestamp);

		//rank
		rank = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);

		//printf("%d\n", rank);

		//videourl_len
		videourl_len = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);

		if (videourl_len > MAX_VIDEOURL_LEN)
		{
			printf("videourl[%d] too long.", videourl_len);
			return -6;
		}

		//videourl
		memcpy(videourl, p, videourl_len);
		p += videourl_len;
		p_len += videourl_len;
		//printf("%d %s\n", videourl_len, videourl);

		if (p_len > len)
		{
			printf("length not match.[%d] [%d]", p_len, len);
			return -100;
		}

		return p_len;
	}

	int Initi(const char* buf, const int len)
	{
		const char *p = buf;
		int p_len = 0;

		/*word_len*/
		unsigned int word_len = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);
		if (word_len > MAX_WORD_LENGTH)
		{
			printf("word too long.");
			return -1;
		}

		/*word*/
		memcpy(word, p, word_len);
		p += word_len;
		p_len += word_len;
		printf("\nword: %s\n", word);

		//docid
		docid = *(unsigned long long*)p;
		p += sizeof(unsigned long long);
		p_len += sizeof(unsigned long long);
		printf("docid %llu\n", docid);

		//type
		type = *(int*)p;
		p += sizeof(int);
		p_len += sizeof(int);
		printf("type %d\n", type);

		//titleLen
		title_len = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);

		if (title_len > MAX_TITLE_LEN)
		{
			printf("title[%d] too long.", title_len);
			return -1;
		}

		//title
		memcpy(title, p, title_len);
		p += title_len;
		p_len += title_len;

		printf("title %d %s\n", title_len, title);

		//url_len
		url_len = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);

		if (url_len > MAX_URL_LEN)
		{
			printf("url[%d] too long.", url_len);
			return -2;
		}

		//url
		memcpy(url, p, url_len);
		p += url_len;
		p_len += url_len;

		printf("url %d %s\n", url_len, url);

		//sitename_len
		sitename_len = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);

		if (sitename_len > MAX_SITENAME_LEN)
		{
			printf("sitename[%d] too long.", sitename_len);
			return -3;
		}

		//sitename
		memcpy(sitename, p, sitename_len);
		p += sitename_len;
		p_len += sitename_len;

		printf("sitename %d %s\n", sitename_len, sitename);

		//imageurl_len
		imageurl_len = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);

		if (imageurl_len  > MAX_IMAGEURL_LEN)
		{
			printf("imageurl[%d] too long.", imageurl_len);
			return -4;
		}

		//imageurl
		memcpy(imageurl, p, imageurl_len);
		p += imageurl_len;
		p_len += imageurl_len;

		//img_width
		img_width = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);

		//img_height
		img_height = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);

		printf("image %d %s %d %d\n", imageurl_len, imageurl, img_width, img_height);

		//summary_len
		summary_len = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);

		if (summary_len  > MAX_SUMMARY_LEN)
		{
			printf("summary[%d] too long.", summary_len);
			return -5;
		}

		//summary
		memcpy(summary, p, summary_len);
		p += summary_len;
		p_len += summary_len;

		printf("summary %d %s\n", summary_len, summary);

		//src_timestamp
		src_timestamp = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);

		printf("src_timestamp %d\n", src_timestamp);

		//in_timestamp
		in_timestamp = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);
		printf("%d\n", in_timestamp);

		//rank
		rank = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);

		printf("%d\n", rank);

		//videourl_len
		videourl_len = *(unsigned int*)p;
		p += sizeof(unsigned int);
		p_len += sizeof(unsigned int);

		if (videourl_len > MAX_VIDEOURL_LEN)
		{
			printf("videourl[%d] too long.", videourl_len);
			return -6;
		}

		//videourl
		memcpy(videourl, p, videourl_len);
		p += videourl_len;
		p_len += videourl_len;
		printf("%d %s\n", videourl_len, videourl);

		if (p_len > len)
		{
			printf("length not match.[%d] [%d]", p_len, len);
			return -100;
		}

		return p_len;
	}

public:
	char word[MAX_WORD_LENGTH];		/*result对应的词*/

	unsigned long long docid;		/*DocId号*/
	unsigned int type;						/*条目类型*/
	unsigned int title_len;					/*标题长度*/
	char title[MAX_TITLE_LEN];		/*标题*/
	unsigned int url_len;					/*url长度*/
	char url[MAX_URL_LEN];			/*url*/
	unsigned int sitename_len;				/*来源站点长度*/
	char sitename[MAX_URL_LEN];		/*来源站点*/
	unsigned int imageurl_len;				/*缩略图url长度*/
	char imageurl[MAX_IMAGEURL_LEN];/*缩略图url*/
	unsigned int img_width;
	unsigned int img_height;
	unsigned int summary_len;				/*摘要长度*/
	char summary[MAX_SUMMARY_LEN];	/*摘要*/
	unsigned int src_timestamp;				/*原文时间戳*/
	unsigned int in_timestamp;				/*入库时间戳*/
	unsigned int rank;						/*相关性排序序号*/
	unsigned int videourl_len;				/*视频预览url长度*/
	char videourl[MAX_IMAGEURL_LEN];/*视屏预览url*/
};


#endif /* KSSSEARCHRESULT_H_ */
