 #include<stdio.h>
#include<stdlib.h>
#include <time.h>
 
#define MAX_EVENTS 10000
#define MAX_VERTICES 1000   //we intially define MAX_VERTICES to be 1000
#define MAX_EDGES 10        //we intially define MAX_EDGES to be 10
#define TIME_MAX 300        //we intially define TIME_MAX to be 300
/*creating a structure that is mean't to store the events in the priority queue
it stores the person to whom the event is happening,the process that is happening (Transmission or Recovery) and the time that person got infected)*/
struct queue{
	int person_no;
	char event;
	int time;
	int infected_time;
};
struct queue *pq[MAX_EVENTS];//declaring the priority queue as a global variable
int size;   //keeping track of the size of the queue
/*this structure is used to store the list containing the people who come in a respective category and 
their infected time and recovery time if required for calculating any average*/
struct list{
	int person;
	int infected_time;   //in the case that the time is '-1' it implies that the time is expected to be infinity as it has not been predicted yet
	int recovery_time;
	struct list *next;
};
/*this structure is used to depict a database that contains the 3 lists and also the priority queue*/
struct data{
	struct list *Susceptible;
	struct list *Infected;
	struct list *Recovered;
};
//this structure is used to store the neighbours of a given vertex 
struct edge{
	int person_numb;
	struct edge *neighbours;
};
//this structure depicts a single vertex in the node and it stores the status of the node and neighbours as well as number of neighbours of the node
struct adj_list{
	int person_num;
	char status;
	int number_of_neighbours;
	struct edge *neighbour;
};
struct adj_list *connections[MAX_VERTICES];   //declaring a array of pointers to store the data of the vertices to create a adjacency list
void network();        //declaring a function network
void insert1();        //declaring a function insert1
void insert2(int vertex1,int vertex2);   //declaring a function insert2
int search(struct adj_list *vertex1,int vertex2);  //declaring a function search
void Enqueue(int Person_no,char process,int time,int Infected_time);  //declaring a function Enqueue
void Dequeue();    //declaring a function Dequeue
void heapify(int index); //declaring a function heapify
struct list* Insert(struct list *List,int Person,int Infected_time,int Recovery_time); //declaring a function Insert
struct list* Delete(struct list *List,int Person);  //declaring a function Delete
struct data* SIR(struct data* Database);            //declaring a function SIR
struct data* Transmission(struct data* Database);   //declaring a function Transmission
struct data* Recovery(struct data* Database);       //declaring a function Recovery
int nodes(struct list *List);                       //declaring a function nodes
int Search(struct list *List,int Person);           //declaring a function Search
void printlist(struct list *List);                  //declaring a function printlist
int main() 
{
	srand(time(NULL));                //we seed the rand function
	size=0;
	network();                       //using the network function to create a randomised adjacency list 
	struct data *database;            //DDeclaing a pointer database of type struct data and allocating memory to it
	database=(struct data*)malloc(sizeof(struct data));
	database->Infected=NULL;
	database->Recovered=NULL;
	database->Susceptible=NULL;
	database=SIR(database);        //running the SIR simulation to generate a result of the pandemic
	return 0;
}
//the nodes function takes a list as input and calculates the number of people in that list and returns this value
int nodes(struct list *List)  
{
	struct list *temp;
	temp=List;
	int i=0;
	while(temp!=NULL)
	{
		i++;
		temp=temp->next;
	}
	return i;
} 
/*the SIR function is used  to simulate a pandemic of the SIR type for a time of TIME_MAX*/
struct data* SIR(struct data* Database)
{
	int i;
	//initially we add all the people to the susceptible list
	for(i=1;i<=MAX_VERTICES;i++)
	{
		if(Database->Susceptible==NULL)
		{
			Database->Susceptible=Insert(Database->Susceptible,i,-1,-1);
		}
		else
		{
			Insert(Database->Susceptible,i,-1,-1);
		}
	}
	//randomly generate  the number of people who are initially infected and keep it <=3
	int check=rand()%3+1;
	int j;
	int p;
	for(j=0;j<check;j++)
	{
		p=(rand()%1000)+1;   //generating the IDs of the random people who are infected at the beginning
		if(connections[p-1]->status=='S')   //this is a checking case which sees if the person to be infected is susceptible
		{
			Enqueue(p,'T',0,0);    //adding the transmission event to the queue
			if(Database->Infected==NULL)  //in the case that the infected list is empty
			{
				Database->Infected=Insert(Database->Infected,p,0,-1);  //we add the person to the infected list
				if(Database->Susceptible->person==p)    //deleting the person from the susceptible list
				{
					Database->Susceptible=Delete(Database->Susceptible,p);
				}
				else
				{
					Delete(Database->Susceptible,p);
				}
			}
			else
			{
				Insert(Database->Infected,p,0,-1);                //we add the person to the infected list
				if(Database->Susceptible->person==p)            //deleting the person from the susceptible list
				{
					Database->Susceptible=Delete(Database->Susceptible,p);
				}
				else
				{
					Delete(Database->Susceptible,p);
				}
			}
		}
	}
	int day=0; //define a integer day and equate it to 0
	while(size>0)  //proceeding with the events in the queue as long as the queue is not empty
	{
		while(pq[0]->time-day>=0)      //printing the day the number of people in each list at every day till the day when the queue is empty
		{
			printf("\nDAY %d :",day);  //printing the contents of the various list on every day
			printf("\nSusceptible List :");
			printlist(Database->Susceptible);
			printf("\nInfected List :");
			printlist(Database->Infected);
			printf("\nRecovered List :");
			printlist(Database->Recovered);
			day++;
		}
		if(pq[0]->event=='T')  //in the case that the event to be done is transmission
		{
			Database=Transmission(Database);
		}
		else                             //in the case that the event to be done is Recovery
		{
			Database=Recovery(Database);
		}
	}
	printf("\nDAY %d[END] :",day);  //printing the contents of the various list on every day
	printf("\nSusceptible List :");
	printlist(Database->Susceptible);
	printf("\nInfected List :");
	printlist(Database->Infected);
	printf("\nRecovered List :");
	printlist(Database->Recovered);
	return Database;
}
/*the transmission function is used to generate the time at which we expect the infected node to get cured and 
add this event to the queue as well as to determine when the neighbours of the infected get infected in the case that they do get infected*/
struct data* Transmission(struct data* Database)
{
	int time;
	int rec_time;
	//in the case that a person was infected by another person at a earlier time than the event in the priority queue
	if(connections[pq[0]->person_no-1]->status=='I'||connections[pq[0]->person_no-1]->status=='R')
	{
		Dequeue();
		return Database;
	}
	connections[pq[0]->person_no-1]->status='I';//assing the status of infected to the person undergoing transmission
	if(Database->Infected==NULL)  //adding the person undergoing transmission to the infected list
	{
		Database->Infected=Insert(Database->Infected,pq[0]->person_no,pq[0]->time,-1);
	}
	else
	{
		Insert(Database->Infected,pq[0]->person_no,pq[0]->time,-1);
	}
	if(Database->Susceptible->person==pq[0]->person_no) //removing the person undergoing transmission from the susceptible list
	{
		Database->Susceptible=Delete(Database->Susceptible,pq[0]->person_no);
	}
	else
	{
		Delete(Database->Susceptible,pq[0]->person_no);
	}
	rec_time=pq[0]->time+1;   //starting the recovery time 1 day after the person got infected
	while(rec_time<TIME_MAX)
	{
		//estimating the time taken for a person to recover by coin toss method and giveing a probability of 0.2 of getting cured 
		if(rand()%10<2)  
		{
			//adding the recovery event to the queue
			Enqueue(pq[0]->person_no,'R',rec_time,pq[0]->time);
			break;	
		}
		else
		{
			rec_time++;
		}
	}
	struct edge *temp1;
	temp1=connections[pq[0]->person_no-1]->neighbour;
	while(temp1!=NULL)  //checking for all the neighbours of the infected node of the estimated time when they get infected in the case they get infected
	{
		//only in the case that the neighbour is susceptible then only we allow him to have the chance of getting infected
		if(connections[temp1->person_numb-1]->status=='S') 
		{
			//alowwing the neighbour to get infected by the source starting from a time 1 greater than the time when the source wwas infected
			time=pq[0]->time+1; 
			//only as long as the time taken to get infected is less than the time taken to recover by the source then only we allow the person to get infected
			while(time<rec_time) 
			{
				if(rand()%10<5)  //allowing the person to get infected with a probability of 0.5
				{
					//adding the transmission event to the queue
					Enqueue(temp1->person_numb,'T',time,time);
					break;
				}
				else
				{
					time++;
				}
			}
		}
		temp1=temp1->neighbours;
	}
	Dequeue();//removing the event from the priority queue  once completed
	return Database;
}
/*the recovery function is used to change the position of the person once he/she has recovered*/
struct data* Recovery(struct data* Database)
{
	connections[pq[0]->person_no-1]->status='R';//we change the status of the person to recovered
	if(Database->Recovered==NULL)   //adding the person to the recovered list of people
	{
		Database->Recovered=Insert(Database->Recovered,pq[0]->person_no,pq[0]->infected_time,pq[0]->time);
	}
	else
	{
		Insert(Database->Recovered,pq[0]->person_no,pq[0]->infected_time,pq[0]->time);
	}
	if(Database->Infected!=NULL)  //removing the person from the infected list of people
	{
		if(Database->Infected->person==pq[0]->person_no)
		{
			Database->Infected=Delete(Database->Infected,pq[0]->person_no);
		}
		else
		{
			Delete(Database->Infected,pq[0]->person_no);
		}
	}
	Dequeue();//removing the event from the priority queue once completed
	return Database;
}
/*the Search function is used to search for a particular person in a given list of people*/
int Search(struct list *List,int Person)
{
	struct list *temp;
	temp=List;
	while(temp!=NULL)
	{
		if(temp->person==Person)
		{
			return 1;
		}
		else
		{
			temp=temp->next;
		}
	}
	return 0;
}
/*the network function is used to generate a randomised undirected adjacency list*/
void network()
{
	int maxNumberOfEdges = rand()%MAX_EDGES+1;  //we select the max number of edges to be a random number between 1 and the MAX_EDGES
	insert1();                     //we use the insert1 function to add all the vertices to the adjacency list
	int edge_counter;
	int vertex_counter;
	for(vertex_counter=1;vertex_counter<=MAX_VERTICES;vertex_counter++) //generating edges for all vertices
	{
		//ensuring that the edges added are less than max number of edges and also accounting for the edges already present in the graph 
		for(edge_counter=connections[vertex_counter-1]->number_of_neighbours;edge_counter<maxNumberOfEdges;edge_counter++)
		{
			if(rand() % 2==1)    //adding a edge with a probability of 0.5
			{
				int linkedvertex=(rand() % MAX_VERTICES)+1;   //generating a random vertex and linking it to the given vertex using insert2 
				//in the case the vertex to be connected to has more than the max number of edges then we don't add the edge
				if(connections[linkedvertex-1]->number_of_neighbours<maxNumberOfEdges)
				{
					insert2(vertex_counter,linkedvertex);
				}
			}
		}
	}
}
/*the insert1 function is used to assign all the vertices(people) to the pointers in the global variable collections*/
void insert1()
{
	struct adj_list *temp;
	int i;
	for(i=1;i<=MAX_VERTICES;i++)
	{
		temp=(struct adj_list *)malloc(sizeof(struct adj_list));
		temp->person_num=i;
		temp->number_of_neighbours=0; //keeping the initial number of neighbours to be 0 and its initial status to be susceptible
		temp->status='S';
		temp->neighbour=NULL;
		connections[i-1]=temp;
	}
}
/*the insert2 function is used  to establish a link between vertex1 and vertex2 as well as between vvertex2 and vertex1*/
void insert2(int vertex1,int vertex2)
{
	struct edge *temp;
	struct edge *temp1;
	if(vertex1==vertex2) //in the case that the nodes to be connected are the same
	{
		return;
	}
	//only if the vertex to be connected is not already connected to the given vertex then only we establish the link
	if(search(connections[vertex1-1],vertex2)==0)  
	{
		connections[vertex1-1]->number_of_neighbours++; //increasing the negbours of vertex1 by 1
		temp=(struct edge *)malloc(sizeof(struct edge));
		temp->person_numb=vertex2;
		temp->neighbours=NULL;
		temp1=connections[vertex1-1]->neighbour;
		if(temp1==NULL)
		{
			connections[vertex1-1]->neighbour=temp;
		}
		else
		{
			while(temp1->neighbours!=NULL)
			{
				temp1=temp1->neighbours;
			}
			temp1->neighbours=temp;
		}
		insert2(vertex2,vertex1); //establishing a backward link makin it a undirected graph
	}
}
/*the search function is used to check the neighbour of a given vertex if the other vertex is already a neighbour to it*/
int search(struct adj_list *vertex1,int vertex2)
{
	struct edge *temp;
	temp=vertex1->neighbour;
	while(temp!=NULL)
	{
		if(temp->person_numb==vertex2)
		{
			return 1;
		}
		else
		{
			temp=temp->neighbours;
		}
	}
	return 0;
}
/*the enqueue function is used to add an event to the priority queue*/
void Enqueue(int Person_no,char process,int time,int Infected_time)
{
	if(size>=10000)
	{
		return;
	}
	struct queue *temp;
	temp=(struct queue *)malloc(sizeof(struct queue));
	temp->person_no=Person_no;
	temp->event=process;
	temp->infected_time=Infected_time;
	temp->time=time;
	struct queue *temp1;
	if(size==0)          //in the case that the queue is empty
	{
		pq[0]=temp;
		size++;         //increasing size by 1
	}
	else
	{
		pq[size]=temp;      //adding the event to the end of the queue and the checking with the parent links to get its correct position
		int i;
		i=size;
		while(1)
		{
			if(i==0)
			{
				break;
			}
			if(pq[(i-1)/2]->time>pq[i]->time)
			{
				temp1=pq[i];
				pq[i]=pq[(i-1)/2];
				pq[(i-1)/2]=temp1;
				i=(i-1)/2;
			}
			else
			{
				break;
			}
		}
		size++;    //increasing size by 1
	}
	
}
//the Dequeue function is used to remove an event from the queue once it is completed
void Dequeue()
{
	struct queue* temp;
	temp=pq[0];
	pq[0]=pq[size-1];
	pq[size-1]=NULL;
	size--;   //decreasing size by 1
	heapify(0);   //ensuring that the heap property of the binary tree is maintained
	free(temp);
}
//the heapify function is used to ensure that the heap property of the binary tree is maintained in the priority queue
void heapify(int index)
{
	int lt;
	int rt;
	int least;
	least=index;
	lt=2*index+1;
	rt=2*index+2;
	if(lt<size)
	{
		if(pq[least]->time>pq[lt]->time)
		{
			least=lt;
		}
	}
	if(rt<size)
	{
		if(pq[least]->time>pq[rt]->time)
		{
			least=rt;
		}
	}
	if(least==index)
	{
		return;
	}
	else
	{
		struct queue *temp;
		temp=pq[index];
		pq[index]=pq[least];
		pq[least]=temp;
		heapify(least);
	}
}
//the Insert function is used to add a person to a given list
struct list* Insert(struct list *List,int Person,int Infected_time,int Recovery_time)
{
	struct list *temp;
	struct list *temp1;
	/*in the case that the person to be added is already present in the list when we randomly generate the initally infected people
	 and call the transmission function */
	if(Search(List,Person)==1)  
	{
		return List;
	}
	temp=(struct list *)malloc(sizeof(struct list));
	if(List==NULL)
	{
		temp->person=Person;
		temp->infected_time=Infected_time;   //adding the persons infected and recovery time
		temp->recovery_time=Recovery_time;  //if the time is -1 it implies that the predicted time is infinity
		temp->next=NULL;
		return temp;
	}
	else
	{
		temp1=List;
		while(temp1->next!=NULL)
		{
			temp1=temp1->next;
		}
		temp->person=Person;
		temp->infected_time=Infected_time;  //adding the persons infected and recovery time
		temp->recovery_time=Recovery_time;  //if the time is -1 it implies that the predicted time is infinity
		temp->next=NULL;
		temp1->next=temp;
		return temp1;
	}
}
//the delete function is used to remove a person from a given list of people
struct list* Delete(struct list *List,int Person)
{
	struct list *temp1;
	struct list *temp2=NULL;
	temp1=List;
	if(Search(List,Person)==0)  //in the case that the person to be deleted is not present in the list 
    {
        return List;
    }
	while(temp1->person!=Person)
	{
		temp2=temp1;
		temp1=temp1->next;
	}
	if(temp2==NULL)
	{
		temp2=temp1->next;
		free(temp1);
		return temp2;
	}
	temp2->next=temp1->next;
	free(temp1);
	return temp2;
}
void printlist(struct list *List)
{
	struct list *temp;
	temp=List;
	while(temp!=NULL)
	{
		printf("%d",temp->person);
		if(temp->infected_time!=-1)
		{
			printf("(inf_time : %d)",temp->infected_time);
		}
		if(temp->recovery_time!=-1)
		{
			printf("(rec_time : %d)",temp->recovery_time);
		}
		if(temp->next!=NULL)
		{
			printf(",");
		}
		temp=temp->next;
	}
}
