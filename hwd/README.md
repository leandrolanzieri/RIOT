## To compile device tree
1- Run C preprocessor: `gcc -E -P -x assembler-with-cpp -o board.dts  nucleo-l152re.dts`
2- Compile resulting device tree: `dtc -o board.dtb board.dts`
