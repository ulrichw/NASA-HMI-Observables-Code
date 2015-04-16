''' program to check the QUALITY keyword of the hmi.S_720s records
and print the results                                  '''

import numpy as np, psycopg2

def DBparams():
    DBNAME = 'jsoc'
    SERVER = 'hmidb'
    DBUSER = 'couvidat'
    DRMSPGPORT = '5432'    
    return [DBNAME,SERVER,DBUSER,DRMSPGPORT]

if __name__ == '__main__':

    dbparams=DBparams()
    # Connect to the database (and handle exceptions)
    try:
        conn=psycopg2.connect(database=dbparams[0], user=dbparams[2], host=dbparams[1], port=dbparams[3])
    except:
        print("Unable to connect to the database",dbparams[0])
        sys.exit(1)
    
    cur = conn.cursor()
    cur.execute("SELECT quality,count(quality) AS countq FROM hmi.S_720s WHERE t_rec >= 1207594800.0 GROUP BY quality;")

    quality=[]
    countq=[]

    for record in cur:
        quality.append(record[0])
        countq.append(record[1])

    quality=np.array(quality)
    countq=np.array(countq)    

    print(quality,countq)

    index = np.where(quality == 0)

    print("the percentage of records with a good quality is:",float(countq[index]/np.sum(countq))*100.,"%")

    cur.close()
    conn.close()
