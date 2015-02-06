#include <stdio.h>
#include "jpge.h"


class bigfile_stream : public jpge::output_stream
{
	bigfile_stream(const bigfile_stream &);
	bigfile_stream &operator= (const bigfile_stream &);

   FILE* m_pFile;
   bool m_bStatus;

public:
   bigfile_stream() : m_pFile(NULL), m_bStatus(false) { }

   virtual ~bigfile_stream()
   {
      close();
   }

   bool open(const char *pFilename)
   {
      close();
      m_pFile = fopen(pFilename, "wb");
      m_bStatus = (m_pFile != NULL);
      return m_bStatus;
   }

   bool close()
   {
      if (m_pFile)
      {
         if (fclose(m_pFile) == EOF)
         {
            m_bStatus = false;
         }
      }
      return m_bStatus;
   }

   virtual bool put_buf(const void* pBuf, int len)
   {
      m_bStatus = m_bStatus && (fwrite(pBuf, len, 1, m_pFile) == 1);
      return m_bStatus;
   }

   uint get_size() const
   {
      return m_pFile ? ftell(m_pFile) : 0;
   }
};
