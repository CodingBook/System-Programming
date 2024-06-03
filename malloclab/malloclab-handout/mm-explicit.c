/*
 * mm-explicit.c - an empty malloc package
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 *
 * @id : 202002473 
 * @name : Seunghyeok Kim
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"

/* If you want debugging output, use the following macro.  When you hand
 * in, remove the #define DEBUG line. */
#define DEBUG
#ifdef DEBUG
# define dbg_printf(...) printf(__VA_ARGS__)
#else
# define dbg_printf(...)
#endif


/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif /* def DRIVER */


/* Vars and Prototypes ----------------------------------*/
static char *heap_listp = 0;
static char *free_listp = 0;

int mm_init(void);
static void *extend_heap(size_t words);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
void *malloc (size_t size);
void *coalesce(void *bp);
static void pop_free(void *bp);
static void push_front_free(void *bp);
void free(void *bp);
void *realloc(void *oldptr, size_t size);


/* Macros --------------------------------------*/
#define HDRSIZE 	4			// header size
#define FTRSIZE 	4			// footer size 
#define WSIZE 		4			// word 한개의 크기
#define DSIZE 		8			// double word 한개의 크기(= 최소 payload의 크기)
#define CHUNKSIZE 	(1<<12)		// 초기 heap 크기 계산용 (= 4096)
#define OVERHEAD 	8			// header + footer의 크기, 실제 데이터는 저장되지 않는 공간
#define ALIGNMENT	8			// ALIGN 계산용

#define MAX(x, y)		((x) > (y) ? (x) : (y))			// 둘 중 큰값 반환
#define MIN(x, y)		((x) < (y) ? (x) : (y))			// 둘 중 작은거

#define PACK(size, alloc)	((unsigned)((size) | (alloc)))		// 한 블록에 size와 할당여부 저장 (손쉽게 header, footer 제작)

#define GET(p)			(*(unsigned *)(p))						// p가 가리키는 곳에 word 크기값 반환
#define PUT(p, val) 	(*(unsigned *)(p) = (unsigned)(val))	// p가 가리키는 곳에 word 크기의 val 입력
#define GET8(p)			(*(unsigned long *)(p))							// free block의 주소를 담을 곳이 8사이즈 이므로
#define PUT8(p, val)	(*(unsigned long *)(p) = (unsigned long)(val))	// 8버전 추가로 만들기 

#define GET_SIZE(p)		(GET(p) & ~0x7)				// block의 size 반환 (하위 3비트 제외한 값), (8단위로 증가)
#define GET_ALLOC(p)	(GET(p) & 0x1)				// block의 할당 여부 반환 (하위 1비트), (1이면 할당, 0이면 미할당)


#define HDRP(bp)		((char*)(bp) - WSIZE)							// bp의 header 주소
#define FTRP(bp)		((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)		// bp의 footer 주소(header 정보에 따라 결정)

#define NEXT_BLKP(bp)	((char*)(bp) + GET_SIZE((char*)(bp) - WSIZE))	// bp의 다음 block의 주소
#define PREV_BLKP(bp)	((char*)(bp) - GET_SIZE((char*)(bp) - DSIZE))	// bp의 이전 block의 주소


#define NEXT_FREEP(bp)  ((char*)(bp))             		// 현재 block에서 다음 free block의 주소를 저장한 장소
#define PREV_FREEP(bp)  ((char*)(bp) + DSIZE)     		// 현재 block에서 이전 free block의 주소를 저장한 장소 

#define NEXT_FREE_BLKP(bp) 	((char*)GET8((char*)(bp)))			// 다음 free block을 가리킴
#define PREV_FREE_BLKP(bp) 	((char*)GET8((char*)(bp) + DSIZE))	// 이전 free block을 가리킴


#define ALIGN(p) (((size_t)(p) + (ALIGNMENT-1)) & ~0x7)			// 크기 계산용, 적절한 8의 배수 반환 

/*
 * Initialize: return -1 on error, 0 on success.
 */

/*
 * mm_init - Heap을 초기화, Heap block을 포인터를 선언해 구현
 */
int mm_init(void) {
    if ((heap_listp = mem_sbrk(4*DSIZE)) == NULL)		// 초기 메모리 할당
		return -1;									


	PUT8(heap_listp, NULL);									// NEXT
	PUT8(heap_listp + DSIZE, NULL);							// PREV
	PUT(heap_listp + 2*DSIZE, 0);							// padding block 0
	PUT(heap_listp + 2*DSIZE + WSIZE, PACK(OVERHEAD, 1));	// prologue header
	PUT(heap_listp + 2*DSIZE + DSIZE, PACK(OVERHEAD, 1));	// prologue footer
	PUT(heap_listp + 2*DSIZE + WSIZE + DSIZE, PACK(0, 1));	// epilogue header

	free_listp = heap_listp;	// heap의 맨 앞인 NEXT를 가리킴
	heap_listp += 3*DSIZE;		// prologue header와 footer의 사이 


	if (extend_heap(CHUNKSIZE/WSIZE) == NULL)	// 힙 확장 및 free block 생성
		return -1;

	return 0;
}


/*
 * extend_heap - heap에 공간이 없을때 words 만큼 빈 블록 생성
 */
static void *extend_heap(size_t words) {
	char *bp;		// heap 확장 후 block pointer
	size_t size;	// 확장 할 크기

	/* 여러개의 word 생성하기(words가 홀수면 +1로 짝수로 만들기) */
	size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;

	/* heap에 더 많은 메모리 요청, 실패하면 null */
	if ((long)(bp = mem_sbrk(size)) < 0)
		return NULL;


	PUT(HDRP(bp), PACK(size, 0));		  // 새 free block의 header
	PUT(FTRP(bp), PACK(size, 0));		  // 새 free block의 footer
	PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // 새 epilogue header

	return coalesce(bp);  // 확장 후 필요할 경우 merge
}


/*
 * coalesce - 인접 block의 상태에 따라, 4가지 경우로 block 합병
 */
void *coalesce(void *bp) {
	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));	// 이전 block의 할당 여부
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));	// 다음 block의 할당 여부
	size_t size = GET_SIZE(HDRP(bp));	// 현재 block의 크기

	
	/* case 1 : 이전, 다음 block이 둘다 할당된 경우 => 병합 없음 */
	if (prev_alloc && next_alloc) {
	}

	/* case 2 : 이전은 할당, 다음은 미할당인 경우 => 다음 block과 병합 */
	else if (prev_alloc && !next_alloc) {
		pop_free(NEXT_BLKP(bp));				// 다음 free block을 list에서 제거
		size += GET_SIZE(HDRP(NEXT_BLKP(bp))); 	// 다음 block의 size만큼 증가
		PUT(HDRP(bp), PACK(size, 0));			// size정보가 바뀐 새 header
		PUT(FTRP(bp), PACK(size, 0));			// size정보가 바뀐 새 footer (앞의 새 header에 의해 위치가 자동으로 갱신)
	}

	/* case 3 : 이전이 미할당, 다음은 할당된 경우 => 이전 block과 병합 */
	else if (!prev_alloc && next_alloc) {
		pop_free(PREV_BLKP(bp));				// 이전 free block을 list에서 제거
		size += GET_SIZE(FTRP(PREV_BLKP(bp)));	// 이전 block의 size만큼 증가
		PUT(FTRP(bp), PACK(size, 0));			// footer에 size 수정
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));// 새 header 설정
		bp = PREV_BLKP(bp);						// bp를 갱신  
	}

	/* case 4 : 이전, 다음 둘다 미할당인 경우 => 둘다 병합*/
	else {
		pop_free(PREV_BLKP(bp));				// 이전 free block을 list에서 제거
		pop_free(NEXT_BLKP(bp));				// 다음 free block을 list에서 제거
		size += GET_SIZE(FTRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp))); // 앞뒤 size 합	
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));// 앞 block header에 설정
		PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));// 뒷 block footer에 설정
		bp = PREV_BLKP(bp);						// bp를 갱신
	}

	/* 작업한 free block을 free list에 추가, 및 반환 */
	push_front_free(bp);
	return bp;
}


/*
 * malloc - 입력 사이즈 만큼 find_fit, 할당될 경우 pointer 반환, 안될경우 heap 증가 후 Block 생성
 */
void *malloc (size_t size) {
    size_t asize;		// 할당할 size
	size_t extendsize;	// 맞는 block이 없을 경우 사용할 변수
	char *bp;

	if (size == 0) return NULL;	// 예외처리, 작업할 필요 없을때

	asize = ALIGN(size + 2*DSIZE + OVERHEAD); // 배정할 크기 계산


	/* 넣을 곳을 찾은 뒤, 성공하면 할당 */
	if ((bp = find_fit(asize)) != NULL) {	
		place(bp, asize);					
		return bp;							
	}
	
	/* 실패했을 경우, heap 확장 후 다시시도 */
	extendsize = MAX(asize, CHUNKSIZE);					// 둘 중 큰거(필요한 경우, CHUNKSIZE보다 크게 확장하도록)
	if ((bp = extend_heap(extendsize/WSIZE)) == NULL)	// extend_heap이 실패 했을때, 예외처리
		return NULL;

	place(bp, asize);	// 확장된 heap에 할당
	return bp;	
}


/*
 * find_fit - 할당 가능한 block을 이전부터 검색
 */
static void *find_fit(size_t asize) {
	void *bp;
	
	// first-fit으로 free list를 탐색
    bp = free_listp;
    while (bp != NULL) { 
        if (asize > GET_SIZE(HDRP(bp)))
            bp = NEXT_FREE_BLKP(bp);
		else 
            return bp;
    }

	// 실패한 경우
    return NULL;
}


/*
 * pop_free - free list에서 제거
 */
static void pop_free(void *bp) {
	/* 처음을 제거하는 경우, free_listp를 수정 */
	if (bp == free_listp) {
		PUT8(PREV_FREEP(NEXT_FREE_BLKP(bp)), NULL);
		free_listp = NEXT_FREE_BLKP(bp);
	}
	/* 중간을 제거하는 경우, 현재를 건너뛰게 연결 수정 */
	else {
		PUT8(NEXT_FREEP(PREV_FREE_BLKP(bp)), NEXT_FREE_BLKP(bp));
		if (NEXT_FREEP(bp) != NULL)
			PUT8(PREV_FREEP(NEXT_FREE_BLKP(bp)), PREV_FREE_BLKP(bp));
	}
}


/*
 * push_front_free - free list에 맨 앞에 추가
 */
static void push_front_free(void *bp) {
	PUT8(NEXT_FREEP(bp), free_listp);	// list의 새 요소	
	PUT8(PREV_FREEP(free_listp), bp);	// 기존 free_listp와 연결,
	free_listp = bp;					// 새 요소를 free_listp로
}


/*
 * place - block point에 asize 작성
 */
static void place(void *bp, size_t asize) {
	size_t bpsize = GET_SIZE(HDRP(bp));	// free상태인 bp의 size

	pop_free(bp);	// bp에 할당하므로, bp를 free list에서 제거

	/* 할당할 asize가 bp에 들어가도, 새 block을 만들 만큼 공간이 많이 남는 경우 */
	if ((bpsize-asize) >= 2*DSIZE + OVERHEAD) {
		PUT(HDRP(bp), PACK(asize, 1));  // asize만큼 할당
		PUT(FTRP(bp), PACK(asize, 1));  // 할당 상태를 1로

		bp = NEXT_BLKP(bp);						// 빈공간을 새로운 block으로
		PUT(HDRP(bp), PACK((bpsize-asize), 0));	// 크기를 빈공간에 맞게
		PUT(FTRP(bp), PACK((bpsize-asize), 0));	// 할당 상태는 0
		push_front_free(bp);					// 새 free block을 list에 추가
	}
	/* 아닐경우, 기존 size만큼 할당 */
	else {
		PUT(HDRP(bp), PACK(bpsize, 1));  // bpsize만큼 할당
		PUT(FTRP(bp), PACK(bpsize, 1));  // 할당 상태를 1로
	}
}


/*
 * free - 할당된 메모리 해제
 */
void free(void *bp) {
    if (!bp) // free할 필요 없을때
		return;

	// block의 size 가져오기
	size_t size = GET_SIZE(HDRP(bp));

	// size는 기존과 같게, 할당 상태를 0으로 변경
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));

	// 필요할 경우, 병합
	coalesce(bp);
}


/*
 * realloc - you may want to look at mm-naive.c
 * 기존 point를 free, 새로운 크기의 block 할당
 */
void *realloc(void *oldptr, size_t size) {
	size_t oldsize;
	void *newptr;

	/* If size == 0 then is just free, and we return NULL. */
	if(size == 0) {
		free(oldptr);
		return NULL;
	}

	/* If oldptr is NULL, the this is just malloc. */
	if(oldptr == NULL) {
		return malloc(size);
	}

	newptr = malloc(size);

	/* If realloc() fails the original block is left untouched  */
	if(!newptr) {
		return 0;
	}

	/* Copy the old data. */
	oldsize = GET_SIZE(HDRP(oldptr));
	if(size < oldsize) oldsize = size;
	memcpy(newptr, oldptr, oldsize);

	/* Free the old block. */
	free(oldptr);

	return newptr;
}


/*
 * calloc - you may want to look at mm-naive.c
 * This function is not tested by mdriver, but it is
 * needed to run the traces.
 */
void *calloc (size_t nmemb, size_t size) {
    return NULL;
}


/*
 * Return whether the pointer is in the heap.
 * May be useful for debugging.
 */
static int in_heap(const void *p) {
    return p < mem_heap_hi() && p >= mem_heap_lo();
}

/*
 * Return whether the pointer is aligned.
 * May be useful for debugging.
 */
static int aligned(const void *p) {
    return (size_t)ALIGN(p) == (size_t)p;
}

/*
 * mm_checkheap
 */
void mm_checkheap(int verbose) {
}
