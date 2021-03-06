#include "stdafx.h"
#pragma hdrstop

char* _TrimLeft(char* str)
{
	char* p = str;

	while (*p && (u8(*p) <= u8(' ')))
		p++;

	if (p != str)
	{
		char* t = str;
		for (; *p; t++, p++) *t = *p;
		*t = 0;
	}

	return str;
}

char* _TrimRight(char* str)
{
	char* p = str + xr_strlen(str);

	while ((p != str) && (u8(*p) <= u8(' ')))
		p--;

	*(++p) = 0;
	return str;
}

char* _Trim(char* str)
{
	_TrimLeft(str);
	_TrimRight(str);
	return str;
}

const char* _SetPos(const char* src, u32 pos, char separator)
{
	const char*	res = src;
	u32 p = 0;

	while ((p < pos) && (0 != (res = strchr(res, separator))))
	{
		res++;
		p++;
	}

	return res;
}

const char* _CopyVal(const char* src, char* dst, char separator)
{
	const char*	p;
	size_t	n;
	p = strchr(src, separator);
	n = (p > 0) ? (p - src) : xr_strlen(src);
	strncpy(dst, src, n);
	dst[n] = 0;

	return dst;
}

int	_GetItemCount(const char* src, char separator)
{
	u32 cnt = 0;
	if (src&&src[0])
	{
		const char*	res = src;
		const char*	last_res = res;
		while (0 != (res = strchr(res, separator)))
		{
			res++;
			last_res = res;
			cnt++;
			if (res[0] == separator) break;
		}

		if (xr_strlen(last_res))
			cnt++;
	}
	return cnt;
}

char* _GetItem(const char* src, int index, char* dst, u32 const dst_size, char separator, const char* def, bool trim)
{
	const char*	ptr;
	ptr = _SetPos(src, index, separator);

	if (ptr)
		_CopyVal(ptr, dst, separator);
	else
		xr_strcpy(dst, dst_size, def);

	if (trim)
		_Trim(dst);

	return dst;
}

char* _GetItems(const char* src, int idx_start, int idx_end, char* dst, char separator)
{
	char* n = dst;
	int level = 0;
	for (const char* p = src; *p != 0; p++)
	{
		if ((level >= idx_start) && (level < idx_end))
			*n++ = *p;

		if (*p == separator)
			level++;

		if (level >= idx_end)
			break;
	}
	*n = '\0';
	return dst;
}

u32 _ParseItem(const char* src, xr_token* token_list)
{
	for (int i = 0; token_list[i].name; i++)
		if (!stricmp(src, token_list[i].name))
			return token_list[i].id;

	return u32(-1);
}

u32 _ParseItem(char* src, int ind, xr_token* token_list)
{
	char dst[128];
	_GetItem(src, ind, dst);
	return _ParseItem(dst, token_list);
}

char* _ReplaceItems(const char* src, int idx_start, int idx_end, const char* new_items, char* dst, char separator)
{
	char* n = dst;
	int level = 0;
	bool bCopy = true;
	for (const char* p = src; *p != 0; p++)
	{
		if ((level >= idx_start) && (level < idx_end))
		{
			if (bCopy) {
				for (const char* itm = new_items; *itm != 0;) *n++ = *itm++;
				bCopy = false;
			}
			if (*p == separator)
				*n++ = separator;
		}
		else
			*n++ = *p;

		if (*p == separator)
			level++;
	}
	*n = '\0';
	return dst;
}

char* _ReplaceItem(const char* src, int index, const char* new_item, char* dst, char separator)
{
	char* n = dst;
	int level = 0;
	bool bCopy = true;
	for (const char* p = src; *p != 0; p++)
	{
		if (level == index)
		{
			if (bCopy)
			{
				for (const char* itm = new_item; *itm != 0;)
					*n++ = *itm++;

				bCopy = false;
			}
			if (*p == separator)
				*n++ = separator;
		}
		else
			*n++ = *p;

		if (*p == separator)
			level++;
	}
	*n = '\0';
	return dst;
}

char* _ChangeSymbol(char* name, char src, char dest)
{
	char *sTmpName = name;
	while (sTmpName[0])
	{
		if (sTmpName[0] == src)
			sTmpName[0] = dest;
		sTmpName++;
	}
	return name;
}

xr_string& _ChangeSymbol(xr_string& name, char src, char dest)
{
	for (xr_string::iterator it = name.begin(); it != name.end(); it++)
		if (*it == src)
			*it = xr_string::value_type(dest);

	return  name;
}

void _SequenceToList(LPSTRVec& lst, const char* in, char separator)
{
	int t_cnt = _GetItemCount(in, separator);
	string1024 T;

	for (int i = 0; i < t_cnt; i++)
	{
		_GetItem(in, i, T, separator, 0);
		_Trim(T);
		if (xr_strlen(T))
			lst.push_back(xr_strdup(T));
	}
}

void _SequenceToList(xr_vector<shared_str>& lst, const char* in, char separator)
{
	lst.clear();
	int t_cnt = _GetItemCount(in, separator);
	xr_string	T;
	for (int i = 0; i < t_cnt; i++)
	{
		_GetItem(in, i, T, separator, 0);
		_Trim(T);
		if (T.size())
			lst.push_back(T.c_str());
	}
}

void _SequenceToList(SStringVec& lst, const char* in, char separator)
{
	lst.clear();
	int t_cnt = _GetItemCount(in, separator);
	xr_string	T;
	for (int i = 0; i < t_cnt; i++)
	{
		_GetItem(in, i, T, separator, 0);
		_Trim(T);
		if (T.size())
			lst.push_back(T.c_str());
	}
}

xr_string	_ListToSequence(const SStringVec& lst)
{
	static xr_string out = "";
	if (lst.size())
	{
		out = lst.front();
		for (auto s_it = lst.begin() + 1; s_it != lst.end(); s_it++)
			out += xr_string(",") + (*s_it);
	}
	return out;
}

xr_string& _TrimLeft(xr_string& str)
{
	const char* b = str.c_str();
	const char* p = str.c_str();

	while (*p && (u8(*p) <= u8(' ')))
		p++;

	if (p != b)
		str.erase(0, p - b);

	return str;
}

xr_string& _TrimRight(xr_string& str)
{
	const char* b = str.c_str();
	size_t l = str.length();
	if (l)
	{
		const char* p = str.c_str() + l - 1;

		while ((p != b) && (u8(*p) <= u8(' ')))
			p--;
		if (p != (str + b))	str.erase(p - b + 1, l - (p - b));
	}
	return str;
}

xr_string& _Trim(xr_string& str)
{
	_TrimLeft(str);
	_TrimRight(str);
	return str;
}

const char* _CopyVal(const char* src, xr_string& dst, char separator)
{
	const char*		p;
	ptrdiff_t	n;
	p = strchr(src, separator);
	n = (p > 0) ? (p - src) : xr_strlen(src);
	dst = src;
	dst = dst.erase(n, dst.length());
	return		dst.c_str();
}

const char* _GetItem(const char* src, int index, xr_string& dst, char separator, const char* def, bool trim)
{
	const char*	ptr;
	ptr = _SetPos(src, index, separator);

	if (ptr)
		_CopyVal(ptr, dst, separator);
	else
		dst = def;

	if (trim)
		_Trim(dst);

	return dst.c_str();
}

shared_str	_ListToSequence(const RStringVec& lst)
{
	xr_string		out;
	if (lst.size())
	{
		out = *lst.front();
		for (RStringVec::const_iterator s_it = lst.begin() + 1; s_it != lst.end(); ++s_it)
		{
			out += ",";
			out += **s_it;
		}
	}
	return shared_str(out.c_str());
}