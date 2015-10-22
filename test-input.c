int main(int argc, char *argv[])
{
	if (Faking > 0)
		placebo();
}
 
static int f1()
{
 
    if ((Flotsam > 0) && (Jetsam < 0))              
        seasick();
    else if ((Flotsam < 0) || (Jetsam == 100))      
        switch (Jetsam) 
        {
            case 1:                                 
            case 2:                                 
            default:                                
                break;
        }
    else
        for (int i=0;i<MAX;i++)
			fluff();
}
