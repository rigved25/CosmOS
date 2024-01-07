// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/trap.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

// LAB 1: add your command to here...
static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace", "Show the backtrace of the current kernel stack", mon_backtrace },
	{ "show", "linux logo", mon_show },
	{ "showmappings", "show mapping of the provided Virtual Addresses", mon_showmappings },

};

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(commands); i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	uint32_t ebp = read_ebp();
	uint32_t *current_ebp = (uint32_t *)ebp;
	
	cprintf("Stack backtrace:\n");

	do
	{
		struct Eipdebuginfo info;
		if(debuginfo_eip(current_ebp[1], &info)){  // it is always returning zero

			//cprintf("ebp %x eip %x args %08x %08x %08x %08x %08x %s:%s: %d\n", current_ebp, current_ebp[1], current_ebp[2], current_ebp[3], current_ebp[4], current_ebp[5], info.eip_file, info.eip_fn_name, info.eip_line);
		}
		else{
			//cprintf("ebp %x eip %x args %08x %08x %08x %08x %08x\n", current_ebp, current_ebp[1], current_ebp[2], current_ebp[3], current_ebp[4], current_ebp[5]);
		}

		//modifying the output of the function name to remove the offset ex "":F(0,18)" in "test_backtrace:F(0,18)"
		char *functionName = (char *)info.eip_fn_name;
		char *offsetPosition = strchr(functionName, ':');
		if (offsetPosition != NULL) {
			*offsetPosition = '\0';
		}

		cprintf("ebp %x eip %x args %08x %08x %08x %08x %08x\n", current_ebp, current_ebp[1], current_ebp[2], current_ebp[3], current_ebp[4], current_ebp[5], current_ebp[6], info.eip_file, info.eip_line, info.eip_fn_name);
		cprintf("%s:%d: %s+%x\n", info.eip_file, info.eip_line, info.eip_fn_name, current_ebp[1] - info.eip_fn_addr);

		current_ebp = (uint32_t *)current_ebp[0];
	} while (current_ebp != 0x0);
	
	//debuginfo_eip

	return 0;
}

int
mon_show(int argc, char **argv, struct Trapframe *tf){

	cprintf("             [38;2;0;0;0m∎[48;2;0;0;0m[39m        [49m[38;2;0;0;0m▆▆[39m\n");
	cprintf("            [38;2;0;0;0m🭋🬹[48;2;0;0;0m[39m            [49m[38;2;0;0;0m🬹🬾[39m\n");
	cprintf("            [38;2;0;0;0m🮈[48;2;0;0;0m[39m              [49m[38;2;0;0;0m▍[39m\n");
	cprintf("            [38;2;0;0;0m🮈[48;2;0;0;0m[39m [49m[38;2;0;0;0m▀🮌🮈[48;2;0;0;0m[39m  [49m[38;2;0;0;0m▍🭋🬹[39m [38;2;0;0;0m🮉[48;2;0;0;0m[39m   [49m[38;2;0;0;0m▍[39m\n");
	cprintf("            [38;2;0;0;0m🮈[48;2;0;0;0m[39m [49m[38;2;0;0;0m⃨[48;2;0;0;0m[39m   [49m[38;2;0;0;0m▍🮈[48;2;0;0;0m[39m [49m [38;2;0;0;0m🮉[48;2;0;0;0m[39m   [49m[38;2;0;0;0m▍[39m\n");
	cprintf("            [38;2;0;0;0m🮈[48;2;0;0;0m[39m [49m[38;2;253;155;44m🬹[48;2;0;0;0m[38;2;254;150;42m🬹[38;2;255;148;41m🬹🬹[48;2;63;35;10m🬹[48;2;141;81;23m[38;2;255;147;42m🮉[48;2;103;60;17m▋[48;2;0;0;0m[38;2;255;148;41m🬹[49m🬹[48;2;0;0;0m[38;2;169;98;27m🬓[39m   [49m[38;2;0;0;0m▍[39m\n");
	cprintf("            [38;2;0;0;0m🮈[48;2;0;0;0m[39m [48;2;10;7;1m[38;2;253;155;44m🮂[48;2;103;62;17m[38;2;255;147;42m🮉[48;2;255;148;42m[38;2;255;148;41mԃԃԃԃ[48;2;104;60;17m[38;2;255;147;42m▋[48;2;11;6;1m[38;2;255;148;41m🮂[48;2;255;148;42mԃ[48;2;226;131;37m[38;2;0;0;0m🮉[48;2;0;0;0m[39m   [49m[38;2;0;0;0m▍[39m\n");
	cprintf("           [38;2;0;0;0m⃨🮈[48;2;0;0;0m[39m  [48;2;54;31;8m[38;2;254;147;40m▝[48;2;70;40;11m[38;2;211;124;35m▚[48;2;1;0;0m[38;2;254;157;45m🬹[48;2;2;1;0m[38;2;253;147;41m🬹[48;2;95;55;15m[38;2;255;148;41m▝[48;2;255;147;42m[38;2;0;0;0m🬹🬹[48;2;0;0;0m[38;2;252;252;252m🬹[38;2;167;167;167m🬓[39m    [49m[38;2;0;0;0m▋[39m\n");
	cprintf("           [38;2;0;0;0m[48;2;0;0;0m[39m   [48;2;30;30;30m[38;2;216;216;216m[48;2;81;62;39m[48;2;12;8;3m[38;2;254;157;45m🮂[48;2;10;5;1m[38;2;253;147;40m🮂[48;2;80;59;38m[38;2;214;214;214m[48;2;243;243;243m[38;2;0;0;0m🮂[48;2;244;244;244m🮂[48;2;254;254;254m[38;2;252;252;252m🮂[48;2;226;226;226m[38;2;2;2;2m🮉[48;2;0;0;0m[39m    [49m[38;2;0;0;0m▆[39m\n");
	cprintf("        [38;2;0;0;0m🝙[48;2;0;0;0m[39m    [48;2;253;253;253m[38;2;255;255;255m🬹🮉[48;2;255;255;255m[39m        [48;2;28;28;28m[38;2;255;255;255m▋[48;2;0;0;0m[39m      [49m[38;2;0;0;0m▍⃮[39m\n");
	cprintf("        [38;2;0;0;0m🮈[48;2;0;0;0m[39m   [48;2;1;1;1m[38;2;0;0;0m▋🮂[48;2;255;255;255m[39m          [48;2;157;157;157m[38;2;255;255;255m▙[48;2;244;244;244m[38;2;0;0;0m🮂[48;2;0;0;0m[39m      [49m[38;2;0;0;0m▋[39m\n");
	cprintf("      [38;2;0;0;0m͙🮉[48;2;0;0;0m[39m    [48;2;226;226;226m[38;2;0;0;0m▋[48;2;255;255;255m[39m             [48;2;0;0;0m[38;2;254;254;254m🬹[38;2;169;169;169m🬓[39m      [49m\n");
	cprintf("      [38;2;0;0;0m▆[48;2;0;0;0m[39m    [48;2;30;30;30m[38;2;216;216;216m[48;2;248;248;248m[38;2;85;85;85m🮂[48;2;255;255;255m[39m             [48;2;254;254;254m[38;2;254;254;254m🮂[48;2;226;226;226m[38;2;0;0;0m🮉[48;2;0;0;0m[39m      [49m[38;2;0;0;0m▆𝕃[39m\n");
	cprintf("    [38;2;0;0;0m🭋🬹[48;2;0;0;0m[39m    [38;2;1;1;1m🬹[48;2;28;28;28m[38;2;255;255;255m🮉[48;2;255;255;255m[39m               [48;2;200;200;200m[38;2;3;3;3m▝[48;2;18;18;18m[38;2;255;255;255m🬓[48;2;0;0;0m[39m      [49m[38;2;0;0;0m▍[39m\n");
	cprintf("    [38;2;0;0;0m🮈[48;2;0;0;0m[39m     [48;2;244;244;244m[38;2;1;1;1m🮂[48;2;158;158;158m[38;2;255;255;255m▟[48;2;255;255;255m[39m                [48;2;28;28;28m[38;2;255;255;255m▋[48;2;0;0;0m[39m      [49m[38;2;0;0;0m▍[39m\n");
	cprintf("    [38;2;0;0;0m🮈[48;2;0;0;0m[39m     [48;2;255;255;255m[38;2;138;138;138m🬹[38;2;177;177;177m🬓[39m              [38;2;0;0;0m🬹[48;2;151;151;151m🮉[48;2;0;0;0m[39m       [49m[38;2;0;0;0m▍[39m\n");
	cprintf("   [38;2;0;0;0m[48;2;125;97;0m▋[48;2;202;158;0m🮂[48;2;244;180;0m🮂[48;2;151;111;0m🮉[48;2;0;0;0m[39m [48;2;5;5;5m[38;2;138;138;138m🮂[48;2;8;8;8m[38;2;216;216;216m🮂[48;2;224;224;224m[38;2;38;38;38m[48;2;255;255;255m[39m           [48;2;252;252;252m[38;2;255;255;255m▋[48;2;250;250;250m🮂[48;2;0;0;0m[39m [48;2;24;18;0m[38;2;173;134;0m[48;2;0;0;0m[39m       [49m[38;2;0;0;0m[39m\n");
	cprintf("[38;2;0;0;0m🬹🬹[48;2;0;0;0m[38;2;255;188;0m🬹[48;2;8;6;0m🬹[48;2;202;149;0m[38;2;12;9;0m▘[48;2;255;188;0m[38;2;255;187;0mЖЖЖ[48;2;103;75;0m[38;2;255;188;0m▋[48;2;0;0;0m[38;2;253;185;0m🬹[39m    [48;2;255;255;255m[38;2;0;0;0m🬹[38;2;85;85;85m🬓[39m        [48;2;28;28;28m[38;2;255;255;255m▋[48;2;1;1;1m[38;2;0;0;0m🬹[48;2;0;0;0m[39m [48;2;28;20;0m[38;2;255;187;0m🮉[48;2;103;76;0m[38;2;255;188;0m▋[48;2;0;0;0m🬹🬹🬹[48;2;200;147;0m[38;2;0;0;0m▘[48;2;255;188;0m[38;2;255;187;0mЖЖ[48;2;200;147;0m[38;2;0;0;0m▝[48;2;16;12;0m[38;2;255;188;0m🬓[49m[38;2;0;0;0m🝙[39m [38;2;0;0;0m🮉[39m\n");
	cprintf("[48;2;151;111;0m[38;2;0;0;0m▋[48;2;244;180;0m🮂[48;2;255;188;0m[38;2;255;187;0mЖЖЖЖЖЖ[48;2;254;186;0m▙[48;2;254;187;0m[38;2;253;185;0m🮂[48;2;0;0;0m[39m     [48;2;28;28;28m[38;2;255;255;255m🮉[48;2;255;255;255m[39m        [48;2;28;28;28m[38;2;255;255;255m▋[48;2;0;0;0m[39m  [48;2;28;20;0m[38;2;255;187;0m🮉[48;2;255;188;0mЖЖЖЖЖЖЖЖ[48;2;28;20;0m[38;2;255;188;0m▋[48;2;0;0;0m[39m [49m [38;2;0;0;0m🮉[39m\n");
	cprintf("[48;2;226;167;0m[38;2;0;0;0m▋[48;2;255;188;0m[38;2;255;187;0mЖЖЖЖЖЖЖЖЖЖ[48;2;199;147;0m[38;2;0;0;0m▝[48;2;16;12;0m[38;2;254;189;0m🬓[48;2;0;0;0m[39m  [48;2;28;28;28m[38;2;255;255;255m🮉[48;2;255;255;255m[39m       [48;2;238;238;238m[38;2;7;7;7m🬦[48;2;57;57;57m[38;2;255;255;255m▘[48;2;0;0;0m[39m [48;2;31;22;0m[38;2;255;188;0m🬹[48;2;122;90;0m[38;2;255;187;0m🮉[48;2;255;188;0mЖЖЖЖЖЖЖЖЖЖ[48;2;0;0;0m[39m  [49m\n");
	cprintf("[48;2;226;167;0m[38;2;0;0;0m▋[48;2;255;188;0m[38;2;255;187;0mЖЖЖЖЖЖЖЖЖЖ[48;2;254;188;0m[38;2;254;188;0m🮂[48;2;28;20;0m▋[48;2;0;0;0m[39m  [48;2;6;6;6m[38;2;170;170;170m🮂[48;2;10;10;10m[38;2;255;255;255m🮂🮂[48;2;220;220;220m🮂[48;2;151;151;151m🮂[48;2;81;81;81m🮂[48;2;10;10;10m🮂🮂[48;2;98;98;98m[38;2;1;1;1m▟[48;2;0;0;0m[38;2;5;5;5m🮂[39m [48;2;255;188;0m[38;2;255;187;0mЖЖЖЖЖЖЖЖЖ[48;2;254;188;0m[38;2;253;193;0m[48;2;103;77;0m[38;2;253;192;0m▋[48;2;10;7;0m[38;2;255;188;0m🮂[49m[38;2;0;0;0m🮂[48;2;0;0;0m[39m [49m\n");
	cprintf("[48;2;151;111;0m[38;2;0;0;0m▋[48;2;255;187;0m🬹🬹🬹[38;2;43;33;0m🬹[38;2;129;99;0m🬹[48;2;255;188;0m[38;2;255;187;0mЖЖЖЖЖЖЖЖ[48;2;0;0;0m[39m            [48;2;255;188;0m[38;2;255;187;0mЖЖЖЖЖЖ[48;2;250;185;0m[38;2;56;41;0m🭋[48;2;255;187;0m[38;2;0;0;0m🬹[49m🮄🮄▋[39m\n");
	cprintf("[38;2;0;0;0m▕🮂🮂🮂[38;2;43;33;0m🮂[38;2;129;99;0m🮂[48;2;10;7;0m[38;2;255;188;0m🮂🮂[48;2;217;162;0m[38;2;42;34;0m🮴[48;2;174;141;0m[38;2;255;188;0m🮂[48;2;255;188;0m[38;2;255;187;0mЖЖ[48;2;103;76;0m[38;2;255;188;0m▋[48;2;10;7;0m🮂[49m[38;2;0;0;0m🮂🮂🮂🮂🮂🮂🮂🮂🮂🮂▜[48;2;0;0;0m[39m [48;2;255;188;0m[38;2;255;187;0mЖЖЖЖ[48;2;230;181;0m[38;2;255;188;0m🮂[48;2;215;163;0m[38;2;36;28;0m[48;2;6;4;0m[38;2;170;125;0m🮂[49m[38;2;0;0;0m🮂[39m\n");
	cprintf("[38;2;198;216;163m🬹[38;2;196;214;161m🬹[38;2;196;214;162m🬹[38;2;199;216;165m🬹[38;2;201;218;167m🬹[38;2;202;220;168m🬹🬹🬹[38;2;201;220;168m🬹[38;2;199;219;167m🬹[48;2;0;0;0m[38;2;195;220;164m🬹[38;2;197;223;166m🬹[38;2;199;227;171m🬹[49m[38;2;202;233;177m🬹[38;2;202;236;182m🬹[38;2;191;225;171m🬹[38;2;182;218;164m🬹[38;2;176;213;160m🬹[38;2;166;208;153m🬹[38;2;164;205;151m🬹[38;2;165;204;150m🬹[38;2;167;203;151m🬹[38;2;169;202;151m🬹[38;2;169;204;150m🬹[38;2;169;206;150m🬹[38;2;170;206;151m🬹[48;2;0;0;0m[38;2;180;217;161m🬹[38;2;171;209;153m🬹[38;2;164;203;147m🬹[38;2;160;201;144m🬹[38;2;164;208;150m🬹[49m[38;2;83;102;75m▙[38;2;183;224;165m🬹[38;2;184;225;165m🬹[38;2;176;215;154m🬹[38;2;165;204;143m🬹[38;2;161;200;138m🬹[38;2;165;203;140m🬹[38;2;166;204;141m🬹[48;2;70;84;59m[38;2;188;212;155m🮉[49m[39m\n");

	return 0;
}

int
mon_showmappings(int argc, char **argv, struct Trapframe *tf){
	// for (i = NPDENTRIES/2; i < NPDENTRIES; i++) {
	// 	pte_t *page_table = (pte_t *)KADDR(PTE_ADDR(pgdir[i]));

	// 	switch (i) {
	// 		case PDX(UVPT):
	// 			cprintf("pgdir[i] & PTE_P - PDX(UVPT): %08x UVPT: %08x - at: ", PDX(UVPT), UVPT);
	// 			break;
	// 		case PDX(KSTACKTOP-1):
	// 			cprintf("pgdir[i] & PTE_P - PDX(KSTACKTOP-1): %08x KSTACKTOP-1: %08x - at: ", PDX(KSTACKTOP-1), KSTACKTOP-1);
	// 			break;
	// 		case PDX(UPAGES):
	// 			cprintf("pgdir[i] & PTE_P - PDX(UPAGES): %08x UPAGES: %08x - at: ", PDX(UPAGES), UPAGES);
	// 			break;
	// 		default: 
	// 			if (i >= PDX(KERNBASE)) {
	// 				cprintf("i >= PDX(KERNBASE) - pgdir[i] & PTE_P | PTE_W - at:  %d ", i);
	// 			} else
	// 				cprintf("i < PDX(KERNBASE) - PDE == 0 - at: %d ", i);
	// 			break;
	// 	}

	// 	cprintf("%d PDE: %08x  RANGE: %08x - %08x\n", i, pgdir[i], page_table[0], page_table[1023]);
	// }
	return 0;
}


/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < ARRAY_SIZE(commands); i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	mon_show(0, 0, 0);
	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");

	if (tf != NULL)
		print_trapframe(tf);

	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
