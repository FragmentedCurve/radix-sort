#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#define BYTES (UCHAR_MAX+1)

static u_char nth_byte(size_t mybyte, int pos)
{
	return (mybyte >> pos * CHAR_BIT) & UCHAR_MAX;
}

static void do_counts(size_t* counts, const size_t* data, size_t data_len, size_t pos)
{
	memset(counts, 0, sizeof (size_t) * BYTES);
	for (size_t i = 0; i < data_len; i++) {
		u_char c = nth_byte(data[i], pos);
		assert(c < BYTES);
		counts[c]++;
	}
}

static void do_prefix_sum(size_t* counts)
{
	for (size_t i = 1; i < BYTES; i++)
		counts[i] += counts[i - 1];
}

size_t* radix_sort(const size_t* data, size_t data_len)
{
	assert(data);
	size_t counts[BYTES];
	size_t* rbuf = malloc(sizeof(size_t) * data_len);
	size_t* wbuf = malloc(sizeof(size_t) * data_len);
	
	if (!rbuf)
		return NULL;

	if (!wbuf) {
		free(rbuf);
		return NULL;
	}

	memcpy(rbuf, data, sizeof(size_t) * data_len);

	for (size_t pos = 0; pos < sizeof(size_t); pos++) {
		do_counts(counts, rbuf, data_len, pos);
		do_prefix_sum(counts);

		if (counts[0] == data_len)
			break;
		
		size_t tail = data_len;
		while (tail > 0) {
			u_char c = nth_byte(rbuf[--tail], pos);
			size_t index = --counts[c];
			wbuf[index] = rbuf[tail];
		}

		size_t* temp = wbuf;
		wbuf = rbuf;
		rbuf = temp;
	}

	free(wbuf);
	return rbuf;
}

#ifdef _UNIT_TEST

#include <time.h>

#define TEST_SIZE 100000
#define COLS 5

static void print_data(const char* prefix, const size_t* data, size_t len)
{
	printf("%s:\n", prefix);
	for (size_t i = 0; i < len; i++) {
		printf("%21zu ", data[i]);
		if ((i+1) % COLS == 0) {
			puts("");
		}
	}
	puts("");
}

static int cmp_byte(const void *a, const void *b)
{
	size_t n = *((size_t*) a);
	size_t m = *((size_t*) b);

	if (n < m)
		return -1;
	else if (n > m)
		return 1;
	return 0;
}

int main()
{
	size_t radix_data[TEST_SIZE];
	size_t qsort_data[TEST_SIZE];
	size_t* output;
	time_t qstart, qend, rstart, rend;

	puts("GENERATING DATA");
	srand(time(NULL));
	for (size_t i = 0; i < TEST_SIZE; i++) {
		radix_data[i] = rand() & 0xFFFFFFFF;
		qsort_data[i] = radix_data[i];
	}

	print_data("RAW DATA", radix_data, TEST_SIZE);
	puts("SORTING DATA WITH RADIX");
	rstart = clock();
	output = radix_sort(radix_data, TEST_SIZE);
	rend = clock();
	rend -= rstart;
	
	puts("SORTING DATA WITH QSORT FROM LIBC");
	qstart = clock();
	qsort(qsort_data, TEST_SIZE, sizeof(size_t), cmp_byte);
	qend = clock();
	qend -= qstart;
	
	puts("COMPARING RESULTS FROM RADIX AND QSORT");
	for (size_t i = 0; i < TEST_SIZE; i++) {
		if (qsort_data[i] != output[i]) {
			print_data("QSORT RESULT", qsort_data, TEST_SIZE);
			print_data("RADIX RESULT", output, TEST_SIZE);
			fprintf(stderr, "FAILED\n");
			return 1;
		}
	}
	puts("SUCCESS");
	printf("QSORT / RADIX = %f\n", (double)qend / rend);
	return 0;
}
#endif