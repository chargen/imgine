
#include "ImgineConfig.h"

#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>

extern "C" {
#include "editline/readline.h"
#include "histedit.h"
}

#include <iostream>

using boost::char_separator;
using boost::tokenizer;

using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using std::string;

char *prompt_string(EditLine *e) {
    return (char *)"> ";
}

int main(int argc, char *argv[])
{
    // Handle program options.

    int verbosity = 0;

    namespace po = boost::program_options;

    // TODO: Generic options
    po::options_description generic("Generic");
    generic.add_options()
        ("version,V", "print version and exit")
        ("help,h", "print help message and exit")
        ;

    // TODO: Configuration
    po::options_description config("Configuration");
    config.add_options()
        ("optimization", po::value<int>()->default_value(10),
         "optimization level")
        ("include-path,I",
         po::value< vector<string> >()->composing(),
         "include path")
        ("verbose,v", po::value<int>(&verbosity)->implicit_value(1),
         "enable verbosity (optionally specify level)")
        ("debug,d",
         "enable debugging")
        ;

    // TODO: Hidden options
    po::options_description hidden("Hidden");
    hidden.add_options()
        ("input-file", po::value< vector<string> >(),
         "input file")
        ;

    po::options_description cmdline_options("Available options");
    cmdline_options.add(generic).add(config).add(hidden);

    po::options_description config_file_options("Configurable options");
    config_file_options.add(config).add(hidden);

    po::options_description visible_options("Options");
    visible_options.add(generic).add(config);

    po::positional_options_description p;
    p.add("input-file", -1);

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).
                  options(cmdline_options).positional(p).run(), vm);
        po::notify(vm);
    } catch (std::exception &e) {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }

    if (vm.count("version")) {
        cout << Imgine_VERSION << endl;
        return EXIT_SUCCESS;
    }

    if (vm.count("help")) {
        cout << "Usage: imgine [options] [files]" << endl;
        cout << visible_options << endl;
        return EXIT_SUCCESS;
    }

    if (vm.count("optimization")) {
        //cout << "optimization level: " << vm["optimization"].as<int>() << endl;
    }

    if (vm.count("include-path")) {
        vector<string> values = vm["include-path"].as< vector<string> >();
        for (const auto &value : values) {
            cout << "include path: " << value << endl;
        }
    }

    if (vm.count("input-file")) {
        vector<string> values = vm["input-file"].as< vector<string> >();
        for (const auto &value : values) {
            cout << "input file: " << value << endl;
        }
    }

    if (vm.count("verbose")) {
        cout << "verbosity level: " << vm["verbose"].as<int>() << endl;
    }

    if (vm.count("debug")) {
        cout << "debugging enabled" << endl;
    }

    // Initialize EditLine.

    EditLine *el;
    el = el_init(argv[0], stdin, stdout, stderr);
    el_set(el, EL_PROMPT, &prompt_string);
    el_set(el, EL_EDITOR, "emacs");

    History *console_history;
    console_history = history_init();
    if (!console_history) {
        cerr << "history could not be initialized" << endl;
        return EXIT_FAILURE;
    }

    HistEvent ev;
    history(console_history, &ev, H_SETSIZE, 800); // history size
    el_set(el, EL_HIST, history, console_history);

    // Enter console loop.

    bool is_console_reading = true;
    while (is_console_reading) {
        int count = 0;
        const char *line;
        line = el_gets(el, &count);

        if (count) {
            history(console_history, &ev, H_ENTER, line); // add to history

            string text(line);
            tokenizer<char_separator<char> > tokens(text);
            tokenizer<char_separator<char> >::iterator token = tokens.begin();

            if (*token == ":") { // special commands
                token++;

                if (*token == "quit" || *token == "q") {
                    // Quit the program.
                    is_console_reading = false;

                } else {
                    // TODO
                    for (; token != tokens.end(); token++)
                        cout << *token << endl;
                }
            } else { // image commands
                // TODO
                for (; token != tokens.end(); token++)
                    cout << *token << endl;
            }
        }
    }

    history_end(console_history);
    el_end(el);

    return EXIT_SUCCESS;
}
