#!/usr/bin/env python


import subprocess as subproc
import sys
import json



valid_argv1 = ["--help", "-h", "--out"]
valid_argv2 = ["--exec"]
debug = False

compiler_name_prefix = "x86_64-w64-mingw32-"
compiler_absolute_path = "C:/CTools/mingw64-msvcrt/bin/"


def main():
	if not debug:
		sys.tracebacklimit = 0
	
	if len(sys.argv) == 1:
		raise TypeError("Not Enough Arguments were given to argv")
	

	argv1 = str(sys.argv[1])
	valid = False
	for arg in valid_argv1:
		# print("<valid={}> || ( {} ==? {} )".format(valid, tmp0, arg))
		valid = valid or ( argv1.startswith(arg) )
	

	if not valid:
		raise TypeError("Invalid Argument at Arg[1], given was {}, accepts only: {}".format( argv1, " ".join(valid_argv1) ))
	else:
		if not argv1.startswith("--out"):
			print("Usage: \n    compile_commands.py --out=<Path to resulting json> --exec=<'make script command...'> ")
			exit(0)

	

	if len(sys.argv) < 3:
		raise ValueError("--out arg must be followed by --exec arg")
	
	argv2 = str(sys.argv[2])
	if not argv1.startswith("--out="): raise TypeError("Script must start with output file as first argument.\n") 
	if not argv2.startswith("--exec="): raise TypeError("Scripts' second argument must be a Make Script-Command, e.g. 'make -f ...'\n")


	output_file     = argv1.replace("--out=", "") # get rid of prepend
	output_filename = output_file.split('/')[-1]
	exec_command    = argv2.replace("--exec=", "").replace("'", "") # get rid of prepend and '
	print("\nGenerating {}... ( {} )".format(output_filename, output_file))
	
	if debug:
		print("\n---compile_commands.py begin =>")
		print("sys.argv[2]: make command is: '{}'".format(exec_command))
		print("---compile_commands    end")
	
	tmp = exec_command.split(" ")
	tmp = [s for s in tmp if s != ' ' and s != '']
	if debug:
		print("argument list is {}".format(tmp))

	sp = subproc.run(tmp, stdout=subproc.PIPE, shell=True, text=True, check=True)
	if sp.returncode != 0:
		print("Process was terminated. Error Code {}".format(sp.returncode))
		exit(sp.returncode)
	
	if debug:
		print("process ran and retrieved the following output ==>\n\n{}".format(sp.stdout))
	

	stdout_lines = sp.stdout.split("\n")
	compiler_gcc_idx = 0
	compiler_gpp_idx = 1
	compiler_asm_idx = 2

	compilers 	      = ['gcc', 'g++', 'as']
	compilers         = ["".join([compiler_name_prefix, compiler]) for compiler in compilers]
	compile_lines_max = [ 100, 	 100, 	50 ]
	compile_lines_cnt = [ 0,     0,     0  ]
	compile_gen = [ [""] * compile_lines_max[i] for i in range(0, 3) ]


	for line in stdout_lines:
		which_compiler = int(compilers[0] in line) + (compilers[1] in line) * 2 + (compilers[2] in line) * 3
		which_compiler -= 1
		
		if which_compiler == -1: continue

		# compile_gen is an array of 3 elements, each of which is a stack of varying length which we fill with compile_lines_cnt

		# compile_gen[which_compiler] => which list-of-strings do we need, gcc, gpp or as?
		# compile_lines_cnt[which_compiler] => the counter for the list-of-strings =>
			# initialize the appropriate line/string with the line we just sanitized/found.
			# increment the counter for the list-of-strings we used
		compile_gen[which_compiler][compile_lines_cnt[which_compiler]] = line
		compile_lines_cnt[which_compiler] += 1


	if debug:
		print("found lines for the following compilers:")
		print("gcc:\n")
		for i in range(0, compile_lines_cnt[compiler_gcc_idx]):
			print(compile_gen[compiler_gcc_idx][i])
		print("end\n")
		
		print("gpp:\n")
		for i in range(0, compile_lines_cnt[compiler_gpp_idx]):
			print(compile_gen[compiler_gpp_idx][i])
		print("end\n")

		print("asm:\n")
		for i in range(0, compile_lines_cnt[compiler_asm_idx]):
			print(compile_gen[compiler_asm_idx][i])
		print("end\n")
	
	# compile command structure that is expected is as follows:
		# [compiler] [arguments] -I[Include Directory]... src_file.ext -o obj_file_ext.o
	
	abs_path = subproc.run(["pwd"], stdout=subproc.PIPE, shell=True, text=True)
	abs_path = abs_path.stdout.replace('\n', "")


	if debug:
		print("abs path found was {}\n\n".format(abs_path))
	
	map_str_to_json = \
	{
		"arguments": [],
		"directory": abs_path,
		"file": '',
		"output": ''
	}
	list_of_dicts = [{} for _ in range(sum(compile_lines_cnt))]
	list_of_dicts_cnt = 0
	
	tmp_file = ""
	tmp_out = ""
	for compiler in range(0, 3):
		for i in range(0, compile_lines_cnt[compiler]):
			arg_str_list = compile_gen[compiler][i].split(" ")
			arg_str_list[:] = (value for value in arg_str_list if  (value != " " and value != "") )

			if debug:
				print("arglist is {}".format(', '.join(arg_str_list)) )
			# arg_str_list[0] = "".join([compiler_absolute_path, compilers[compiler]])

			tmp_out  = arg_str_list[-1]
			tmp_file = arg_str_list[-3]
			arg_str_list.remove(tmp_file)
			arg_str_list.extend([tmp_file])


			map_str_to_json["arguments"] = arg_str_list
			map_str_to_json["file"]      = "/".join([abs_path, tmp_file])
			map_str_to_json["output"]    = "/".join([abs_path, tmp_out ])

			list_of_dicts[list_of_dicts_cnt] = map_str_to_json.copy()
			list_of_dicts_cnt += 1

	if debug:
		[print(dict) for dict in list_of_dicts]
	

	list_of_dicts.pop(); # last dict is the linking stage and should not be included in the compile_commands.json
	with open(output_file, 'w+') as file:
		json.dump(list_of_dicts, file, indent=4)

	print("[Notice] Created {} file at {}".format(output_filename, output_file))
	exit(0)




if __name__ == "__main__":
	main()
