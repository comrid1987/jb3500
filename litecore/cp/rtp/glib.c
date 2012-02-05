
double drand48()
{

	return (double)rand() / 32767.0;
}


void *g_memdup(const void *data, uint_t data_len)
{
	void *p;

	p = rt_malloc(data_len);
	if (p != NULL)
		memcpy(p, data, data_len);
	return p;
}

