#include <streamprotocol.h>

StreamProtocol* streamprotocol_init(DCB* dcb)
{
    StreamProtocol* rval = NULL;
    if((rval = malloc(sizeof(StreamProtocol))) != NULL)
    {
	rval->owner_dcb = dcb;
	rval->pool = pipepool;
	rval->pipe = NULL;
	spinlock_init(&rval->protocol_lock);
    }
    return rval;
}

/**
 * Initialize a pool of pipes
 * @return on success, a pointer to allocated pipe pool, otherwise NULL
 */
PIPEPOOL* stream_init_pool()
{
    int i;
    PIPEPOOL* pool;

    if((pool = malloc(sizeof(PIPEPOOL))) != NULL)
    {
	if((pool->pool = malloc(sizeof(PIPE) * PIPEPOOL_DEFAULT_SIZE)) != NULL)
	{

	    pool->n_pipes = PIPEPOOL_DEFAULT_SIZE;
	    spinlock_init (&pool->lock);
	    for(i = 0;i<pool->n_pipes;i++)
	    {
		if(pipe(pool->pool[i].pipe) != 0)
		{
		    free(pool->pool);
		    free(pool);
		    pool = NULL;
		    break;
		}
		pool->pool[i].owner = 0;
		pool->pool[i].in_use = false;
		pool->pool[i].pool = pool;
	    }
	}
	else
	{
	    free(pool);
	    pool = NULL;
	}
    }
    return pool;
}

PIPE* stream_get_pipe(PIPEPOOL* pool)
{
  int i;
  PIPE* rval;
  bool have_pipe = false;

  spinlock_acquire (&pool->lock);
  while(!have_pipe)
  {
      for(i = 0;i < pool->n_pipes;i++)
      {
          if(!pool->pool[i].in_use)
          {
              pool->pool[i].in_use = true;
              pool->pool[i].owner = pthread_self ();
              rval = &pool->pool[i];
              have_pipe = true;
              break;
          }
      }
  }
  spinlock_release (&pool->lock);

  return rval;
}

void stream_return_pipe(PIPE* pipe)
{
  spinlock_acquire (&pipe->pool->lock);
  pipe->in_use = false;
  pipe->owner = 0;
  spinlock_release (&pipe->pool->lock);
}