// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pstream.h
 */

#ifndef _PSTREAM_H_
#define _PSTREAM_H_

#include <cstdarg>
#include <cstddef>
#include <stdexcept>

#include "pconfig.h"
#include "pstring.h"
#include "palloc.h"

// -----------------------------------------------------------------------------
// pstream: things common to all streams
// -----------------------------------------------------------------------------

class pstream
{
	P_PREVENT_COPYING(pstream);
public:

	typedef long unsigned pos_type;

	static const pos_type SEEK_EOF = (pos_type) -1;

	pstream(unsigned flags) : m_flags(flags)
	{
	}
	virtual ~pstream()
	{
	}

	bool bad() const { return ((m_flags & FLAG_ERROR) != 0); }
	bool seekable() const { return ((m_flags & FLAG_SEEKABLE) != 0); }

	void seek(pos_type n)
	{
		check_seekable();
		return vseek(n);
	}

	pos_type tell()
	{
		return vtell();
	}

protected:
	virtual void vseek(pos_type n) = 0;
	virtual pos_type vtell() = 0;

	static const unsigned FLAG_EOF = 0x01;
	static const unsigned FLAG_ERROR = 0x02;
	static const unsigned FLAG_SEEKABLE = 0x04;
	static const unsigned FLAG_CLOSED = 0x08;    /* convenience flag */

	bool closed() { return ((m_flags & FLAG_CLOSED) != 0); }

	void set_flag(unsigned flag)
	{
		m_flags |= flag;
	}
	void clear_flag(unsigned flag)
	{
		m_flags &= ~flag;
	}

	void check_not_eof() const
	{
		if (m_flags & FLAG_EOF)
			throw pexception("unexpected eof");
	}

	void check_seekable() const
	{
		if (!(m_flags & FLAG_SEEKABLE))
			throw pexception("stream is not seekable");
	}

	unsigned flags() const { return m_flags; }
private:

	unsigned m_flags;
};

// -----------------------------------------------------------------------------
// pistream: input stream
// -----------------------------------------------------------------------------

class pistream : public pstream
{
	P_PREVENT_COPYING(pistream);
public:

	pistream(unsigned flags) : pstream(flags) {}
	virtual ~pistream() {}

	bool eof() const { return ((flags() & FLAG_EOF) != 0) || bad(); }

	/* this digests linux & dos/windows text files */

	bool readline(pstring &line)
	{
		UINT8 c = 0;
		pstringbuffer buf;
		if (!this->read(c))
		{
			line = "";
			return false;
		}
		while (true)
		{
			if (c == 10)
				break;
			else if (c != 13) /* ignore CR */
				buf += c;
			if (!this->read(c))
				break;
		}
		line = buf;
		return true;
	}

	bool read(UINT8 &c)
	{
		return (read(&c, 1) == 1);
	}

	unsigned read(void *buf, unsigned n)
	{
		return vread(buf, n);
	}

protected:
	/* read up to n bytes from stream */
	virtual unsigned vread(void *buf, unsigned n) = 0;

private:
};

// -----------------------------------------------------------------------------
// postream: output stream
// -----------------------------------------------------------------------------

class postream : public pstream
{
	P_PREVENT_COPYING(postream);
public:

	postream(unsigned flags) : pstream(flags) {}
	virtual ~postream() {}

	/* this digests linux & dos/windows text files */

	void writeline(const pstring &line)
	{
		write(line.cstr(), line.blen());
		write(10);
	}

	void write(const pstring &text)
	{
		write(text.cstr(), text.blen());
	}

	void write(const char c)
	{
		write(&c, 1);
	}

	void write(const void *buf, unsigned n)
	{
		vwrite(buf, n);
	}

protected:
	/* write n bytes to stream */
	virtual void vwrite(const void *buf, unsigned n) = 0;

private:
};

// -----------------------------------------------------------------------------
// pomemstream: output string stream
// -----------------------------------------------------------------------------

class pomemstream : public postream
{
	P_PREVENT_COPYING(pomemstream);
public:

	pomemstream();
	virtual ~pomemstream();

	char *memory() const { return m_mem; }
	unsigned size() const { return m_size; }

protected:
	/* write n bytes to stream */
	virtual void vwrite(const void *buf, unsigned n);
	virtual void vseek(pos_type n);
	virtual pos_type vtell();

private:
	pos_type m_pos;
	pos_type m_capacity;
	pos_type m_size;
	char *m_mem;
};

// -----------------------------------------------------------------------------
// pofilestream: file output stream
// -----------------------------------------------------------------------------

class pofilestream : public postream
{
	P_PREVENT_COPYING(pofilestream);
public:

	pofilestream(const pstring &fname);
	virtual ~pofilestream();

	void close();

protected:
	/* write n bytes to stream */
	virtual void vwrite(const void *buf, unsigned n);
	virtual void vseek(pos_type n);
	virtual pos_type vtell();

private:
	void *m_file;
	pos_type m_pos;
};

// -----------------------------------------------------------------------------
// pifilestream: file input stream
// -----------------------------------------------------------------------------

class pifilestream : public pistream
{
	P_PREVENT_COPYING(pifilestream);
public:

	pifilestream(const pstring &fname);
	virtual ~pifilestream();

	void close();

protected:
	/* read up to n bytes from stream */
	virtual unsigned vread(void *buf, unsigned n);
	virtual void vseek(pos_type n);
	virtual pos_type vtell();

private:
	void *m_file;
	pos_type m_pos;
};

// -----------------------------------------------------------------------------
// pimemstream: input memory stream
// -----------------------------------------------------------------------------

class pimemstream : public pistream
{
	P_PREVENT_COPYING(pimemstream);
public:

	pimemstream(const void *mem, const pos_type len);
	pimemstream(const pomemstream &ostrm);
	virtual ~pimemstream();

protected:
	/* read up to n bytes from stream */
	virtual unsigned vread(void *buf, unsigned n);
	virtual void vseek(pos_type n);
	virtual pos_type vtell();

private:
	pos_type m_pos;
	pos_type m_len;
	char *m_mem;
};

// -----------------------------------------------------------------------------
// pistringstream: input string stream
// -----------------------------------------------------------------------------

class pistringstream : public pimemstream
{
	P_PREVENT_COPYING(pistringstream);
public:

	pistringstream(const pstring &str) : pimemstream(str.cstr(), str.len()), m_str(str) { }

private:
	/* only needed for a reference till destruction */
	pstring m_str;
};


#endif /* _PSTREAM_H_ */
