vim: vim.c
	@$(CC) vim.c -o vim.out -Wall -Wextra -pedantic -std=c99
	@./vim.out