#include "lake.h"
const char LakeScript[] = "\n\
--[[ Lua will report the line number \n\
 relative to the string so placing at the top\n\
 simplifies searching for errors --]]\n\
function conditional(result,ontrue,onfalse)\n\
  local func = onfalse\n\
  if result == true then func = ontrue end\n\
  if type(func) == 'function' then\n\
    return func()\n\
  end\n\
  return func\n\
end\n\
function fault(msg,lvl)\n\
	if not lvl then lvl = 2 end\n\
	print(debug.traceback())\n\
	error(msg,lvl + 1)\n\
end\n\
getcwd = Lake.getcwd\n\
mkdir = Lake.mkdir\n\
access = Lake.access\n\
function exists(path) return (access(path,0)==0) end\n\
thedir = Lake.dirname\n\
thefile = Lake.basename\n\
function theext(path)\n\
	local ext = path:match('[^.]*$')\n\
	if not ext then return '' end\n\
	return '.' .. ext\n\
end\n\
setenv = Lake.setenv\n\
getenv = Lake.getenv\n\
Lake.PATH = getenv('PATH')\n\
-- Make sure we can use system('cd ../ && lake')\n\
if not Lake.PATH:match('lake') then\n\
	local d = thedir(Lake.launchpath())\n\
	if not d or d == '' then d = Lake.launchpath() end\n\
	d = tostring(d) .. ';' .. Lake.PATH\n\
	setenv(d)\n\
end\n\
-- Free memory\n\
Lake.PATH = niL\n\
Lake.nsys = {\n\
	bits = Lake.bitsof('*'),\n\
	isWindows = ((getenv('OS') or ''):match('^Windows')),\n\
}\n\
if Lake.nsys.isWindows then\n\
	Lake.nsys.shset = 'set '\n\
	Lake.nsys.arch = getenv('PROCESSOR_ARCHITECTURE')\n\
	Lake.nsys.exe_ext = '.exe'\n\
	Lake.nsys.dll_pfx = ''\n\
	Lake.nsys.dll_ext = '.dll'\n\
	Lake.nsys.flags = ' -mconsole -mwindows -mwin32' ..\n\
	  conditional(Lake.nsys.bits==64,' -m64 -D _WIN64','-m32')\n\
else\n\
	Lake.nsys.shset = 'export '\n\
	Lake.nsys.arch = io.popen'uname -m':read'*a'\n\
	Lake.nsys.exe_ext = '.sh'\n\
	Lake.nsys.dll_pfx = 'lib'\n\
	Lake.nsys.dll_ext = '.so'\n\
	Lake.nsys.flags = ''\n\
end\n\
-- Eg: env,prv,pfx=setenvs(tmp) system(pfx .. cmd) setenvs(prv)\n\
-- or system(cmd,tmpenv)\n\
function setenvs( env )\n\
	local prv = {}\n\
	local pfx = ''\n\
	local set = Lake.nsys.shset\n\
	if not env then\n\
		return {}, {}, ''\n\
	end\n\
	for key,val in pairs(env) do\n\
		prv[key] = getenv(key)\n\
		setenv(key,val)\n\
		pfx = pfx .. set .. key .. '=' .. val .. '\\n'\n\
	end\n\
	return env, prv, pfx\n\
end\n\
function literalize(str)\n\
    return str:gsub('[%(%)%.%%%+%-%*%?%[%]%^%$]', '%%%0')\n\
end\n\
function setsep(path)\n\
	if type(path) ~= 'string' then\n\
		fault('setsep(path) path should be a string!',2)\n\
		return ''\n\
	end\n\
	return path:gsub('/',Lake.dirsep)\n\
end\n\
function system(command,tmpenv)\n\
	local name = os.tmpname() .. '_txt'\n\
	local env,prv,pfx = setenvs(tmpenv)\n\
	name = Lake.getcwd() .. '/' .. name:gsub('[/\\\\]','_')\n\
	--command = pfx .. command .. ' > \"' .. setsep(name) .. '\"'\n\
	command = pfx .. command .. '2>1'\n\
	print( command )\n\
	local r = Lake.system( command )\n\
	if (Lake.access(name,0)~=0) then setenv(prv) return r end\n\
	for line in io.lines(name) do\n\
		print(line)\n\
	end\n\
	os.remove(name)\n\
	setenv(prv)\n\
	return (r or 1)\n\
end\n\
function escchar(text,char)\n\
	return text:gsub('%'..char,'%%'..char)\n\
end\n\
function replace(str,text,with)\n\
	return str:gsub(literalize(text),with)\n\
end\n\
function iwildcard(path)\n\
	local dpath = Lake.dirname(path)\n\
	if not dpath or dpath == '' or dpath == '.' then dpath = './' end\n\
	local dir = LakeDir(dpath)\n\
	if not dir then\n\
		fault( 'iwildcard() ' .. dpath .. ' not opened!' )\n\
		return function() return nil end\n\
	end\n\
	local ent = dir:next()\n\
	if not ent then\n\
		print( 'iwildcard() ' .. dpath .. ' had no first entry!' )\n\
		return function() return nil end\n\
	end\n\
	local i = 0\n\
	path = replace(path,dpath,'')\n\
	path = path:gsub('%.','%%.')\n\
	path = path:gsub('%*','.*')\n\
	return function()\n\
		while ( ent ) do\n\
			if ent.d_name:match(path) then\n\
				local tmp = ent\n\
				ent = dir:next()\n\
				i = i + 1\n\
				return i - 1, tmp\n\
			end\n\
			ent = dir:next()\n\
		end\n\
		-- Just in case dir:next() didn't shut it for us\n\
		dir:shut()\n\
		return nil\n\
	end\n\
end\n\
function wildcard(path)\n\
	local l = ''\n\
	for e, entry in iwildcard(path) do\n\
		l = l .. ';' .. entry.d_name\n\
	end\n\
	return l:sub(2)\n\
end\n\
print('Loading lua file...')\n\
";
