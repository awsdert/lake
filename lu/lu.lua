function printvec(name,vec)
	for i,v in pairs(vec) do
		print( name .. '["' .. tostring(i) .. '"] = "' .. tostring(v) .. '"' )
	end
end
function Library(version,rootdir)
	-- Give 3rd party libraries a chance to define their own needs
	local file = loadfile(rootdir .. "/library")
	if file then return file() end
	-- Fallback if library.lua doesn't exist in provided directory
	return {
		ver = version,
		root = rootdir,
		include = rootdir .. '/include',
		lib = rootdir .. '/lib',
		bin = rootdir .. '/bin',
		src = rootdir .. '/src',
		libs = {},
		cflags = '-I"' .. setsep(rootdir .. '/include')
			.. '" -L"' .. setsep(rootdir .. '/lib') .. '"',
		bflags = '-I"' .. setsep(rootdir .. '/include')
			.. '" -L"' .. setsep(rootdir .. '/lib') .. '"'
	}
end
function MinGW_BatchLibrary(version,rootdir)
	local mgw = Library( version, rootdir )
	-- Informs make to use a batch file before initiating gcc
	mgw.cflags = ''
	mgw.bflags = ''
	mgw.set_paths = function(shell)
		if Lu.access(mgw.bin .. '/gcc.exe') ~= 0 then
			return nil
		elseif Lu.access(mgw.root .. '/git/cmd/git.exe') == 0 then
			shell.env.PATH = mgw.root .. '/git/cmd;' .. shell.env.PATH
		end
		shell.env.X_DISTRO = "nuwen"
		-- simple check for envoriment variable existance
		local function insmgw(var,ins)
			ins = setsep(ins)
			if shell.env[var] then
				if shell.env[var]:match("MinGW") then
					return
				end
				shell.env[var] = ins .. ';' .. shell.env[var]
				return
			end
			shell.env[var] = ins
		end
		local include = mgw.include .. ';' .. mgw.include .. '/freetype2'
		insmgw( "PATH", mgw.bin )
		insmgw( "CPATH", include )
		insmgw( "C_INCLUDE_PATH", include )
		insmgw( "OBJC_INCLUDE_PATH", include )
		insmgw( "CPLUS_INCLUDE_PATH", include )
		return shell
	end
	return mgw
end
local mingw32 = MinGW_BatchLibrary( 32, Lu.mountroot() .. "/Common/MinGW" )
local mingw64 = MinGW_BatchLibrary( 64, Lu.mountroot() .. "/Common/MinGW64" )
function LuaLibrary(pfx,version)
	local lua = Library(version,Lu.mountroot() .. "/Common/lua" .. pfx .. version)
	lua.lib = lua.root
	lua.bin = lua.root
	lua.libs.lua = setsep(lua.root .. "/lua" .. version)
	lua.cflags = ' -I "' .. setsep(lua.include) ..
		'" -L "' .. setsep(lua.lib) .. '" -l lua' .. version
	lua.bflags = ' -I "' .. setsep(lua.include) ..
		'" "' .. setsep(lua.lib .. '/lua' .. version .. '.dll"')
	lua.set_paths = function(shell)
		-- simple check for envoriment variable existance
		local function inslua(var,ins)
			ins = setsep(ins)
			if shell.env[var] then
				if shell.env[var]:match("lua") then
					return
				end
				shell.env[var] = ins .. ';' .. shell.env[var]
				return
			end
			shell.env[var] = ins
		end
		inslua( "PATH", lua.lib .. ';' .. shell.env.PATH )
		inslua( "CPATH", lua.include )
		inslua( "C_INCLUDE_PATH", lua.include )
		inslua( "OBJC_INCLUDE_PATH", lua.include )
		inslua( "CPLUS_INCLUDE_PATH", lua.include )
		return shell
	end
	return lua
end
local lua53 = LuaLibrary("",53)
local lua32_53 = LuaLibrary("32-",53)
local lua64_53 = LuaLibrary("64-",53)
function ismodified(src,dst)
	if (Lu.access(dst)) == 0 then
		local s = Lu.stat(src)
		local d = Lu.stat(dst)
		if d.st_mtime == s.st_mtime then
			return false
		end
	end
	return true
end
local folders = {
	out = {
		path = Lu.mountroot() .. "/Common/",
		sub = {}
	}, obj = {
		path = "obj/",
		sub  = {}
	}
}
folders.out.sub.lu = folders.out.path ..  "lu/"
folders.out.sub.lu32 = folders.out.path ..  "lu32/"
folders.out.sub.lu64 = folders.out.path ..  "lu64/"
folders.obj.sub.win16 = folders.obj.path .. "win16/"
folders.obj.sub.win32 = folders.obj.path .. "win32/"
folders.obj.sub.win64 = folders.obj.path .. "win64/"
local bits = Lu.bitsof("*")
local build_cc = 'gcc --pass-exit-codes -Wall'
local debug_cc = build_cc .. ' -ggdb'
local objs = {
	{ src = "lu.c" }
}
function olist(prepath)
	local l = ""
	for o,obj in pairs(objs) do
		if not obj.dst then obj.dst = obj.src .. '.o' end
		objs[o] = obj
		l = l .. ' ' .. prepath .. obj.dst
	end
	return l
end
local libs = {
	lua = { dst = folders.out.sub.lu .. "lua53.so", src = lua53.root .. "/lua53.so" },
	lua16 = { dst = folders.out.sub.lu .. "lua53.dll", src = lua53.root .. "/lua53.dll" },
	lua32 = { dst = folders.out.sub.lu32 .. "lua53.dll", src = lua32_53.root .. "/lua53.dll" },
	lua64 = { dst = folders.out.sub.lu64 .. "lua53.dll", src = lua64_53.root .. "/lua53.dll" }
}
local shells = {
	sh = {
		env = { PATH = getenv('PATH') },
		cflags = '',--lua53.cflags,
		bflags = ''--lua53.bflags
	},
	sh16 = {
		env = { PATH = getenv('PATH') },
		cflags = ' -mconsole -mwindows -mwin32 -D _WIN16',-- .. lua53.cflags,
		bflags = ' -mconsole -mwindows -mwin32 -D _WIN16'-- .. lua53.bflags
	},
	sh32 = {
		env = { PATH = getenv('PATH') },
		cflags = ' -mconsole -mwindows -mwin32 -m32 -D _WIN32',-- .. lua32_53.cflags,
		bflags = ' -mconsole -mwindows -mwin32 -m32 -D _WIN32'-- .. lua32_53.bflags
	},
	sh64 = {
		env = { PATH = getenv('PATH') },
		cflags = ' -mconsole -mwindows -mwin32 -m64 -D _WIN64',-- .. lua64_53.cflags,
		bflags = ' -mconsole -mwindows -mwin32 -m64 -D _WIN64'-- .. lua64_53.bflags
	}
}
shells.env32 = mingw32.set_paths(shells.sh32)
shells.env32 = lua32_53.set_paths(shells.sh32)
shells.env64 = mingw64.set_paths(shells.sh64)
shells.env64 = lua64_53.set_paths(shells.sh64)
mingw32 = nil
mingw64 = nil
lua32_53 = nil
lua64_53 = nil
local apps = {
	lu = {
		path = folders.out.sub.lu .. "lu.sh",
		prepath = folders.obj.path,
		cc = build_cc,
		cflags = shells.sh.cflags,
		bflags = shells.sh.bflags,
		libs = { libs.lua },
		objs = olist(folders.obj.path),
		env = shells.sh.env
	},
	lu16 = {
		path = folders.out.sub.lu .. "lu.cmd",
		prepath = folders.obj.path,
		cc = build_cc,
		cflags = shells.sh16.cflags,
		bflags = shells.sh16.bflags,
		libs = { libs.lua16 },
		objs = olist(folders.obj.path),
		env = shells.sh16.env
	},
	lu32 = {
		path = folders.out.sub.lu32 .. "lu32.exe",
		prepath = folders.obj.sub.win32,
		cc = build_cc,
		cflags = shells.sh32.cflags,
		bflags = shells.sh32.bflags,
		libs = { libs.lua32 },
		objs = olist(folders.obj.sub.win32),
		env = shells.sh32.env
	},
	lu64 = {
		path = folders.out.sub.lu64 .. "lu64.exe",
		prepath = folders.obj.sub.win64,
		cc = build_cc,
		cflags = shells.sh64.cflags,
		bflags = shells.sh64.bflags,
		libs = { libs.lua64 },
		objs = olist(folders.obj.sub.win64),
		env = shells.sh64.env
	},
	dlu = {
		path = folders.out.sub.lu .. "dlu.sh",
		prepath = folders.obj.path .. 'd',
		cc = debug_cc,
		cflags = shells.sh.cflags,
		bflags = shells.sh.bflags,
		libs = { libs.lua },
		objs = olist(folders.obj.path .. 'd'),
		env = shells.sh.env
	},
	dlu16 = {
		path = folders.out.sub.lu .. "dlu.cmd",
		prepath = folders.obj.path .. 'd',
		cc = debug_cc,
		cflags = shells.sh16.cflags,
		bflags = shells.sh16.bflags,
		libs = { libs.lua16 },
		objs = olist(folders.obj.path .. 'd'),
		env = shells.sh16.env
	},
	dlu32 = {
		path = folders.out.sub.lu32 .. "dlu32.exe",
		prepath = folders.obj.sub.win32 .. 'd',
		cc = debug_cc,
		cflags = shells.sh32.cflags,
		bflags = shells.sh32.bflags,
		libs = { libs.lua32 },
		objs = olist(folders.obj.sub.win32 .. 'd'),
		env = shells.sh32.env
	},
	dlu64 = {
		path = folders.out.sub.lu64 .. "dlu64.exe",
		prepath = folders.obj.sub.win64 .. 'd',
		cc = debug_cc,
		cflags = shells.sh64.cflags,
		bflags = shells.sh64.bflags,
		libs = { libs.lua64 },
		objs = olist(folders.obj.sub.win64 .. 'd'),
		env = shells.sh64.env
	}
}
local T = {}
T.directories = function(list)
	local r, path, mk = Lu.mkdir
	for p,obj in pairs(list) do
		if type(obj) == "table" then path = obj.path
		else path = obj end
		if exists(path) == false then
			echo("mkdir() " .. path)
			r = Lu.mkdir(path)
			if r ~= 0 then fault("Directory " .. path ..
				"couldn't be made with result " .. tostring(r) .. "!")
				return r end
		end
		if type(obj) == "table" then
			r = T.directories(obj.sub)
			if r ~= 0 then fault("Subdirectories could not be made with result " .. tostring(r) .. "!") return r end
		end
	end
	return 0
end
T.libraries = function(bin)
	local r = T.directories(folders)
	if r ~= 0 then fault("T.directories() returned" .. tostring(r) .. "!") return r end
	local cp = Lu.copy
	for l,lib in pairs(bin.libs) do
		if ismodified(lib.src,lib.dst) == true then
			print('copy() ' .. lib.src .. ' > ' .. lib.dst)
			r = cp(lib.src,lib.dst)
			if r ~= 0 then fault("Failed to copy " .. lib.src ..
			" with result " .. tostring(r) .. "!") return r end
		end
	end
	return 0
end
T.objects = function(bin)
	local r = T.libraries(bin)
	if r ~= 0 then fault("T.libraries() returned " .. tostring(r) .. "!") return r end
	for o,obj in pairs(objs) do
		if ismodified(obj.src,obj.dst) then
			r = system( bin.pfx .. bin.cc .. bin.cflags .. ' -c "' .. obj.src ..
				'" -o "' .. bin.prepath .. obj.dst .. '"' )
			if r ~= 0 then
				fault("Object " .. obj.dst ..
				" failed to compile with result " .. tostring(r) .. "!")
				return r
			end
		end
	end
	return 0
end
local lu_exe = 'lu' .. bits
local lu_same = (((Lu.launcharg(0)):match(lu_exe)) ~= nil)
local lu_src = lu_exe ..  '/' .. lu_exe
local lu_dst = lu_exe ..  '/lu'
local dlu_src = lu_exe ..  '/d' .. lu_exe
local dlu_dst = lu_exe ..  '/dlu'
T.binaries = function(list)
	local r, env, prv, pfx
	for b,bin in pairs(list) do
		env, prv, pfx = setenvs(bin.env)
		bin.pfx = pfx
		r = T.objects(bin)
		if r ~= 0 then 
			fault("T.objects() returned " .. tostring(r) .. "!")
			return r
		end
		-- Prevents attempted overriding of currently running lu
		if lu_same == true then
			bin.path = bin.path:gsub(lu_src,lu_dst)
			bin.path = bin.path:gsub(dlu_src,dlu_dst)
		end
		r = system(pfx .. bin.cc .. bin.bflags .. bin.objs .. ' -o ' .. bin.path)
		setenvs(prv)
		if r ~= 0 then
			fault("Command return " .. tostring(r) .. "!")
			return r
		end
	end
	return 0
end
T.binaries({
--
apps['lu' .. bits],
apps['dlu' .. bits]
--]]
--[[
apps['lu32'],
apps['dlu32'],
apps['lu64'],
apps['dlu64'],
--]]
})
