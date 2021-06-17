#include <iostream>
using namespace std;





int main(int arg_count, char *args[])
{
    if (arg_count > 1)
    {

        cout << args[1] << endl;
    }
    else
    {
        cout << "Username not supplied.. exiting the program \n";
    }



    
    return 0;
}
