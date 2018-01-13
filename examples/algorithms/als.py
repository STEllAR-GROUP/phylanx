import numpy as np

def ALS(ratings, regularization, num_factors, iterations, alpha=40):
    num_users=np.shape(ratings)[0]
    num_items=np.shape(ratings)[1]

    conf=alpha*ratings
    np.random.seed(0)
    X=np.random.rand(num_users, num_factors)
    Y=np.random.rand(num_items, num_factors)
    I_f=np.identity(num_factors)
    I_i=np.identity(num_items)
    I_u=np.identity(num_users)

    for k in range(iterations):
        YtY=np.dot(Y.T,Y)
        XtX=np.dot(X.T,X)
        for u in range(num_users):
            conf_u=conf[u,:]
            c_u=np.diag(conf_u)
            p_u=conf_u.copy()
            p_u[p_u!=0]=1
            A=YtY+np.dot(np.dot(Y.T,c_u),Y)+regularization*I_f
            b=np.dot(np.dot(Y.T,c_u+I_i),p_u.T)
            X[u,:]=np.dot(np.linalg.inv(A),b)

        for i in range(num_items):
            conf_i=conf[:,i]
            c_i=np.diag(conf_i)
            p_i=conf_i.copy()
            p_i[p_i!=0]=1
            A=XtX+np.dot(np.dot(X.T,c_i),X)+regularization*I_f
            b=np.dot(np.dot(X.T,c_i+I_u),p_i.T)
            Y[i,:]=np.dot(np.linalg.inv(A),b)
    return X,Y


ratings=np.zeros((10,5))
ratings[0,1]=4
ratings[1,0]=1
ratings[1,2]=4
ratings[1,4]=5
ratings[2,3]=2
ratings[3,1]=8
ratings[4,2]=4
ratings[6,4]=2
ratings[7,0]=1
ratings[8,3]=5
ratings[9,0]=1
ratings[9,3]=2

X,Y=ALS(ratings, 0.1, 3,5)
