#include <BtTorrent.h>
#include <QDebug>
#include <getopt.h>

int main(int argc, char *argv[])
{
    bool output_flag = false, input_flag = false;
    QString fileName, ofileName;
    int choice;
    while (1)
    {
        static struct option long_options[] =
        {
            /* Use flags like so:
            {"verbose",	no_argument,	&verbose_flag, 'V'}*/
            /* Argument styles: no_argument, required_argument, optional_argument */
            {"version", no_argument,	0,	'v'},
            {"help",	no_argument,	0,	'h'},
            {"input",	required_argument,	0, 'i'},
            {"output",  required_argument, 0, 'o'},

            {0,0,0,0}
        };

        int option_index = 0;

        /* Argument parameters:
            no_argument: " "
            required_argument: ":"
            optional_argument: "::" */

        choice = getopt_long( argc, argv, "vhi:o:",
                    long_options, &option_index);

        if (choice == -1)
            break;

        switch( choice )
        {
            case 'i':
                fileName = optarg;
                input_flag = true;
                break;
            case 'o':
                ofileName = optarg;
                output_flag = true;
                break;
            case 'v':

                break;

            case 'h':

                break;

            case '?':
                /* getopt_long will have already printed an error */
                break;

            default:
                /* Not sure how to get here... */
                return EXIT_FAILURE;
        }
    }

    /* Deal with non-option arguments here */
    if ( optind < argc )
    {
        while ( optind < argc )
        {

        }
    }

    BtQt::BtTorrent t;
    QFile file(fileName);
    qDebug() << "Decode start...";
    if(t.decodeTorrentFile(file)) {
        qDebug() << "Decode succeed!";
    } else {
        qDebug() << "Decode failed!";
    }
    t.display();
    qDebug() << input_flag << output_flag;

    if(input_flag && output_flag) {
        QFile file(ofileName);
        qDebug() << "Encode start...";
        if(t.encodeTorrentFile(file)) {
            qDebug() << "Encode succeed!";
        } else {
            qDebug() << "Encode failed!";
        }
    }

    return 0;
}
