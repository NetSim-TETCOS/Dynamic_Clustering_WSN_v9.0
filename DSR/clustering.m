function [A,B,C] = clustering(x,scount,num_cls)


	[IDX,C]=kmeans(x,num_cls);
	cl_count=zeros(1,num_cls);
	cl_dist=zeros(1,scount);

% Calculation of Distance from each sensor to the centroid of its cluster and size of each cluster
	
	for index1=1:1:num_cls
		for index2=1:1:scount

			if IDX(index2)==index1
				cl_count(index1)=cl_count(index1)+1;
				cl_dist(index2)=distance(C(index1),C(num_cls+index1),x(index2,1),x(index2,2));% hard coded to be modified
			end

        
		end
	end


	cl_index=zeros(1,num_cls);
	c_head=zeros(1,num_cls);

% Cluster Head election

	for index1=1:1:num_cls
    
		prev_dist=1000;
		for index2=1:1:scount    
		 if IDX(index2)==index1 
			 cl_index(index1)=cl_index(index1)+1;
        
			 if(min(prev_dist,cl_dist(index2))== cl_dist(index2))
				c_head(index1)= index2;
				prev_dist=cl_dist(index2);
			 end

		 end

		end
   
	end


% Graph plotting starts here

	close
	color=char('r','b','m','g','y','c','k');

	for index=1:1:num_cls
	exp=strcat('plot(x(IDX==',num2str(index),',1),x(IDX==',num2str(index),',2),''',color(rem(index,7)),'.'',''MarkerSize'',12)');
	eval(exp);
	exp='hold on';
	eval(exp)
	end

	plot(C(:,1),C(:,2),'ko',...
		 'MarkerSize',12,'LineWidth',2)

	exp='legend(';

	for index=1:1:num_cls
	exp=strcat(exp,'''Cluster ',num2str(index),''',');
	end
	exp=strcat(exp,'''Centroids'',''Location'',''NW'')');
	eval(exp);

%Graph plotting ends here   
   
	A=c_head;    % contains the cluster head id's of each cluster
	B=IDX;	     % contains the cluster id's of each sensor
	C=cl_count;  % contains the size of each cluster



end

